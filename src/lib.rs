//! A rust wrapper for the
//! [Signalsmith Stretch](https://github.com/Signalsmith-Audio/signalsmith-stretch)
//! audio stretching and pitch-shifting library.

#[allow(non_camel_case_types)]
mod sys {
    include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
}

/// Provide a way to track clone initialization type.
/// This is used to ensure that the correct C API function is called when cloning
#[derive(Debug, Clone)]
enum InitType {
    PresetDefault(i32, f32, bool),
    PresetCheaper(i32, f32, bool),
    Configured(i32, i32, i32, bool),
}

/// Provides time stretching and/or pitch shifting for audio.
#[derive(Debug)]
pub struct Stretch {
    inner: *mut sys::stretch_t,
    channel_count: usize,
    seed: Option<i32>,
    callback: Option<Box<fn(f32) -> f32>>,
    init_type: InitType,
}

impl Clone for Stretch {
    fn clone(&self) -> Self {
        let ptr = unsafe {
            if let Some(seed) = self.seed {
                sys::stretch_create_seed(seed as i32)
            } else {
                sys::stretch_create()
            }
        };

        unsafe {
            match self.init_type {
                InitType::PresetDefault(channel_count, sample_rate, split) => {
                    sys::stretch_preset_default(ptr, channel_count, sample_rate, split);
                }
                InitType::PresetCheaper(channel_count, sample_rate, split) => {
                    sys::stretch_preset_cheaper(ptr, channel_count, sample_rate, split);
                }
                InitType::Configured(channel_count, block_sample, interval_sample, split) => {
                    sys::stretch_configure(
                        ptr,
                        channel_count,
                        block_sample,
                        interval_sample,
                        split,
                    );
                }
            }
        }

        Stretch {
            inner: ptr,
            channel_count: self.channel_count,
            seed: self.seed,
            callback: self.callback.clone(),
            init_type: self.init_type.clone(),
        }
    }
}

unsafe impl Send for Stretch {}
unsafe impl Sync for Stretch {}

impl Stretch {
    pub fn new() -> Self {
        let ptr = unsafe { sys::stretch_create() };

        Stretch {
            inner: ptr,
            channel_count: 0,
            seed: None,
            callback: None,
            init_type: InitType::PresetDefault(0, 0.0, false),
        }
    }

    pub fn new_with_seed(seed: i32) -> Self {
        let ptr = unsafe { sys::stretch_create_seed(seed) };

        Stretch {
            inner: ptr,
            channel_count: 0,
            seed: Some(seed),
            callback: None,
            init_type: InitType::PresetDefault(0, 0.0, false),
        }
    }

    pub fn preset_default(
        &mut self,
        channel_count: i32,
        sample_rate: f32,
        split: bool,
    ) {
        if channel_count <= 0 {
            panic!("channel_count must be positive");
        }

        if sample_rate <= 0.0 {
            panic!("sample_rate must be positive");
        }

        unsafe {
            sys::stretch_preset_default(self.inner, channel_count, sample_rate, split);
        }
        self.channel_count = channel_count as usize;
        self.init_type = InitType::PresetDefault(channel_count, sample_rate, split);
    }

    pub fn preset_cheaper(
        &mut self,
        channel_count: i32,
        sample_rate: f32,
        split: bool,
    ) {
        if channel_count <= 0 {
            panic!("channel_count must be positive");
        }

        if sample_rate <= 0.0 {
            panic!("sample_rate must be positive");
        }

        unsafe {
            sys::stretch_preset_cheaper(self.inner, channel_count, sample_rate, split);
        }
        self.channel_count = channel_count as usize;
        self.init_type = InitType::PresetCheaper(channel_count, sample_rate, split);
    }

    pub fn configure(
        &mut self,
        channel_count: i32,
        block_sample: i32,
        interval_sample: i32,
        split: bool,
    ) {
        if channel_count <= 0 {
            panic!("channel_count must be positive");
        }

        if block_sample <= 0 || interval_sample <= 0 {
            panic!("block_sample and interval_sample must be positive");
        }

        unsafe {
            sys::stretch_configure(
                self.inner,
                channel_count,
                block_sample,
                interval_sample,
                split,
            );
        }
        self.channel_count = channel_count as usize;
        self.init_type = InitType::Configured(channel_count, block_sample, interval_sample, split);
    }

    pub fn reset(&mut self) {
        unsafe {
            sys::stretch_reset(self.inner);
        }
    }

    pub fn input_latency(&self) -> i32 {
        unsafe { sys::stretch_input_latency(self.inner) }
    }

    pub fn output_latency(&self) -> i32 {
        unsafe { sys::stretch_output_latency(self.inner) }
    }

    pub fn block_samples(&self) -> i32 {
        unsafe { sys::stretch_block_samples(self.inner) }
    }

    pub fn interval_samples(&self) -> i32 {
        unsafe { sys::stretch_interval_samples(self.inner) }
    }

    pub fn split_computation(&self) -> bool {
        unsafe { sys::stretch_split_computation(self.inner) }
    }

    pub fn set_transpose_semitoses(&mut self, semitones: f32, tonality_limit: Option<f32>) {
        unsafe {
            sys::stretch_set_transpose_semitones(self.inner, semitones, tonality_limit.unwrap_or(0.0));
        }
    }

    pub fn set_transpose_factor(&mut self, factor: f32, tonality_limit: Option<f32>) {
        unsafe {
            sys::stretch_set_transpose_factor(self.inner, factor, tonality_limit.unwrap_or(0.0));
        }
    }

    pub fn set_formant_factor(&mut self, factor: f32, compensate_pitch: bool) {
        unsafe {
            sys::stretch_set_formant_factor(self.inner, factor, compensate_pitch);
        }
    }

    pub fn set_formant_semitones(&mut self, semitones: f32, compensate_pitch: bool) {
        unsafe {
            sys::stretch_set_formant_semitones(self.inner, semitones, compensate_pitch);
        }
    }

    pub fn set_formant_base_frequency(&mut self, frequency: f32) {
        unsafe {
            sys::stretch_set_formant_base(self.inner, frequency);
        }
    }

    pub fn seek(&mut self, input: &[f32], input_sample_count: i32, playback_rate: f32) {
        if input.len() < (input_sample_count as usize * self.channel_count) as usize {
            panic!("Input buffer is smaller than input_sample_count");
        }

        unsafe {
            sys::stretch_seek(
                self.inner,
                &input[0] as *const f32,
                input_sample_count,
                playback_rate as f64,
            );
        }
    }

    pub fn flush(&mut self, output: &mut [f32], output_sample_count: i32, playback_rate: f32) {
        if output.len() < (output_sample_count as usize * self.channel_count) as usize {
            panic!("Output buffer is smaller than output_sample_count");
        }

        unsafe {
            sys::stretch_flush(
                self.inner,
                &mut output[0] as *mut f32,
                output_sample_count,
                playback_rate as f64,
            );
        }
    }

    pub fn seek_length(&self) -> i32 {
        unsafe { sys::stretch_seek_length(self.inner) }
    }

    pub fn output_seek_length(&self, playback_rate: f32) -> i32 {
        unsafe { sys::stretch_output_seek_length(self.inner, playback_rate) }
    }

    pub fn output_seek(&mut self, input: &[f32], input_length: i32) {
        if input.len() < (input_length as usize * self.channel_count) as usize {
            panic!("Input buffer is smaller than input_length");
        }

        unsafe {
            sys::stretch_output_seek(
                self.inner,
                &input[0] as *const f32,
                input_length,
            );
        }
    }

    pub fn process(&mut self, input: &[f32], pcm_length: i32, output: &mut [f32], pcm_out_length: i32) {
        if input.len() < (pcm_length as usize * self.channel_count) as usize {
            panic!("Input buffer is smaller than pcm_length");
        }

        if output.len() < (pcm_out_length as usize * self.channel_count) as usize {
            panic!("Output buffer is smaller than pcm_out_length");
        }

        unsafe {
            sys::stretch_process(
                self.inner,
                &input[0] as *const f32,
                pcm_length,
                &mut output[0] as *mut f32,
                pcm_out_length,
            );
        }
    }

    pub fn exact(&mut self, input: &[f32], pcm_length: i32, output: &mut [f32], pcm_out_length: i32) -> bool {
        if input.len() < (pcm_length as usize * self.channel_count) as usize {
            panic!("Input buffer is smaller than pcm_length");
        }

        if output.len() < (pcm_out_length as usize * self.channel_count) as usize {
            panic!("Output buffer is smaller than pcm_out_length");
        }

        unsafe {
            sys::stretch_exact(
                self.inner,
                &input[0] as *const f32,
                pcm_length,
                &mut output[0] as *mut f32,
                pcm_out_length,
            )
        }
    }
}

impl Drop for Stretch {
    fn drop(&mut self) {
        unsafe { sys::stretch_delete(self.inner) }
    }
}

#[derive(Debug)]
pub struct FFT {
    inner: *mut sys::fft_t,
}

impl FFT {
    pub fn new() -> Self {
        let ptr = unsafe { sys::fft_create() };

        FFT { inner: ptr }
    }

    pub fn resize(&mut self, size: usize) {
        unsafe {
            sys::fft_resize(self.inner, size);
        }
    }

    pub fn size(&self) -> usize {
        unsafe { sys::fft_size(self.inner) as usize }
    }

    pub fn steps(&self) -> usize {
        unsafe { sys::fft_steps(self.inner) as usize }
    }

    pub fn process(&mut self, input: &[num::complex::Complex<f32>], output: &mut [num::complex::Complex<f32>]) {
        unsafe {
            sys::fft_proc(
                self.inner,
                input.as_ptr() as *const sys::complex_t,
                output.as_mut_ptr() as *mut sys::complex_t,
            );
        }
    }

    pub fn process_step(&mut self, step: usize, input: &[num::complex::Complex<f32>], output: &mut [num::complex::Complex<f32>]) {
        unsafe {
            sys::fft_proc_step(
                self.inner,
                step,
                input.as_ptr() as *const sys::complex_t,
                output.as_mut_ptr() as *mut sys::complex_t,
            );
        }
    }

    pub fn process_split(&mut self, input_real: &[f32], input_imag: &[f32], output_real: &mut [f32], output_imag: &mut [f32]) {
        unsafe {
            sys::fft_proc_split(
                self.inner,
                input_real.as_ptr(),
                input_imag.as_ptr(),
                output_real.as_mut_ptr(),
                output_imag.as_mut_ptr(),
            );
        }
    }

    pub fn process_split_step(&mut self, step: usize, input_real: &[f32], input_imag: &[f32], output_real: &mut [f32], output_imag: &mut [f32]) {
        unsafe {
            sys::fft_proc_split_step(
                self.inner,
                step,
                input_real.as_ptr(),
                input_imag.as_ptr(),
                output_real.as_mut_ptr(),
                output_imag.as_mut_ptr(),
            );
        }
    }

    pub fn inverse(&mut self, input: &[num::complex::Complex<f32>], output: &mut [num::complex::Complex<f32>]) {
        unsafe {
            sys::fft_inverse_proc(
                self.inner,
                input.as_ptr() as *const sys::complex_t,
                output.as_mut_ptr() as *mut sys::complex_t,
            );
        }
    }

    pub fn inverse_step(&mut self, step: usize, input: &[num::complex::Complex<f32>], output: &mut [num::complex::Complex<f32>]) {
        unsafe {
            sys::fft_inverse_proc_step(
                self.inner,
                step,
                input.as_ptr() as *const sys::complex_t,
                output.as_mut_ptr() as *mut sys::complex_t,
            );
        }
    }

    pub fn inverse_split(&mut self, input_real: &[f32], input_imag: &[f32], output_real: &mut [f32], output_imag: &mut [f32]) {
        unsafe {
            sys::fft_inverse_proc_split(
                self.inner,
                input_real.as_ptr(),
                input_imag.as_ptr(),
                output_real.as_mut_ptr(),
                output_imag.as_mut_ptr(),
            );
        }
    }

    pub fn inverse_split_step(&mut self, step: usize, input_real: &[f32], input_imag: &[f32], output_real: &mut [f32], output_imag: &mut [f32]) {
        unsafe {
            sys::fft_inverse_proc_split_step(
                self.inner,
                step,
                input_real.as_ptr(),
                input_imag.as_ptr(),
                output_real.as_mut_ptr(),
                output_imag.as_mut_ptr(),
            );
        }
    }
}

impl Drop for FFT {
    fn drop(&mut self) {
        unsafe { sys::fft_delete(self.inner) }
    }
}

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub enum STFTWindowShape {
    Ignore = sys::stft_window_shape_ignore as isize,
    ACG = sys::stft_window_shape_acg as isize,
    Kaiser = sys::stft_window_shape_kaiser as isize,
}

#[derive(Debug)]
pub struct STFT {
    inner: *mut sys::stft_t,
}

impl STFT {
    pub fn new(split_computation: bool) -> Self {
        let ptr = unsafe { sys::stft_create(split_computation) };
        STFT { inner: ptr }
    }

    pub fn configure(
        &mut self,
        in_channel_count: i32,
        out_channel_count: i32,
        block_samples: i32,
        extra_input_history: i32,
        interval_samples: i32,
        asymmetry: f32,
    ) {
        unsafe {
            sys::stft_configure(
                self.inner,
                in_channel_count,
                out_channel_count,
                block_samples,
                extra_input_history,
                interval_samples,
                asymmetry,
            );
        }
    }

    pub fn block_samples(&self) -> usize {
        unsafe { sys::stft_block_samples(self.inner) }
    }

    pub fn fft_samples(&self) -> usize {
        unsafe { sys::stft_fft_samples(self.inner) }
    }

    pub fn bands(&self) -> usize {
        unsafe { sys::stft_bands(self.inner) }
    }

    pub fn reset(&mut self) {
        unsafe {
            sys::stft_reset(self.inner);
        }
    }

    pub fn write_input(&mut self, channel: usize, offset: usize, input: &[f32]) {
        unsafe {
            sys::stft_write_input(
                self.inner,
                channel,
                offset,
                input.len(),
                input.as_ptr(),
            );
        }
    }

    pub fn read_output(&self, channel: usize, offset: usize, output: &mut [f32]) {
        unsafe {
            sys::stft_read_output(
                self.inner,
                channel,
                offset,
                output.len(),
                output.as_mut_ptr(),
            );
        }
    }

    pub fn move_input(&mut self, samples: usize, clear_moved_region: bool) {
        unsafe {
            sys::stft_move_input(self.inner, samples, clear_moved_region);
        }
    }

    pub fn set_interval(&mut self, default_interval: usize, window_shape: STFTWindowShape, asymmetry: f32) {
        unsafe {
            sys::stft_set_interval(self.inner, default_interval, window_shape as i32, asymmetry);
        }
    }

    pub fn analyse(&mut self, sample_in_past: usize) {
        unsafe {
            sys::stft_analyse(self.inner, sample_in_past);
        }
    }

    pub fn synthesise(&mut self) {
        unsafe {
            sys::stft_synthesise(self.inner);
        }
    }

    pub fn spectrum(&self, channel: usize) -> &[f32] {
        unsafe {
            let ptr = sys::stft_spectrum(self.inner, channel);
            std::slice::from_raw_parts(ptr, self.bands())
        }
    }

    pub fn analysis_window(&self) -> &[f32] {
        unsafe {
            let ptr = sys::stft_analysis_window(self.inner);
            std::slice::from_raw_parts(ptr, self.fft_samples())
        }
    }

    pub fn analysis_window_mut(&mut self) -> &mut [f32] {
        unsafe {
            let ptr = sys::stft_analysis_window(self.inner);
            std::slice::from_raw_parts_mut(ptr, self.fft_samples())
        }
    }

    pub fn synthesis_window(&self) -> &[f32] {
        unsafe {
            let ptr = sys::stft_synthesis_window(self.inner);
            std::slice::from_raw_parts(ptr, self.fft_samples())
        }
    }

    pub fn synthesis_window_mut(&mut self) -> &mut [f32] {
        unsafe {
            let ptr = sys::stft_synthesis_window(self.inner);
            std::slice::from_raw_parts_mut(ptr, self.fft_samples())
        }
    }

    pub fn analysis_latency(&self) -> usize {
        unsafe { sys::stft_analysis_latency(self.inner) }
    }

    pub fn synthesis_latency(&self) -> usize {
        unsafe { sys::stft_synthesis_latency(self.inner) }
    }

    pub fn latency(&self) -> usize {
        unsafe { sys::stft_latency(self.inner) }
    }

    pub fn bin_to_freq(&self, bin: f32) -> f32 {
        unsafe { sys::stft_bin_to_freq(self.inner, bin) }
    }

    pub fn freq_to_bin(&self, freq: f32) -> f32 {
        unsafe { sys::stft_freq_to_bin(self.inner, freq) }
    }

    pub fn analyse_steps(&self) -> usize {
        unsafe { sys::stft_analyse_steps(self.inner) }
    }

    pub fn synthesise_steps(&self) -> usize {
        unsafe { sys::stft_synthesise_steps(self.inner) }
    }

    pub fn analyse_step(&mut self, step: usize, sample_in_past: usize) {
        unsafe {
            sys::stft_analyse_step(self.inner, step, sample_in_past);
        }
    }

    pub fn synthesise_step(&mut self, step: usize) {
        unsafe {
            sys::stft_synthesise_step(self.inner, step);
        }
    }

    pub fn samples_since_analysis(&self) -> usize {
        unsafe { sys::stft_samples_since_analysis(self.inner) }
    }

    pub fn samples_since_synthesis(&self) -> usize {
        unsafe { sys::stft_samples_since_synthesis(self.inner) }
    }

    pub fn finish_output(&mut self, strength: f32, offset: usize) {
        unsafe {
            sys::stft_finish_output(self.inner, strength, offset);
        }
    }

    pub fn add_output(&mut self, channel: usize, offset: usize, output: &[f32]) {
        unsafe {
            sys::stft_add_output(
                self.inner,
                channel,
                offset,
                output.len(),
                output.as_ptr(),
            );
        }
    }

    pub fn replace_output(&mut self, channel: usize, offset: usize, output: &[f32]) {
        unsafe {
            sys::stft_replace_output(
                self.inner,
                channel,
                offset,
                output.len(),
                output.as_ptr(),
            );
        }
    }

    pub fn move_output(&mut self, samples: usize) {
        unsafe {
            sys::stft_move_output(self.inner, samples);
        }
    }

    pub fn analysis_offset(&mut self, offset: usize) {
        unsafe {
            sys::stft_analysis_offset(self.inner, offset);
        }
    }

    pub fn synthesis_offset(&mut self, offset: usize) {
        unsafe {
            sys::stft_synthesis_offset(self.inner, offset);
        }
    }
}

impl Drop for STFT {
    fn drop(&mut self) {
        unsafe { sys::stft_delete(self.inner) }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_stretch_functions() {
        let mut stretch = Stretch::new();
        stretch.preset_default(2, 44100.0, false);
        stretch.preset_cheaper(2, 44100.0, false);
        stretch.configure(2, 1024, 256, false);
        stretch.reset();
        let input_latency = stretch.input_latency();
        let output_latency = stretch.output_latency();
        let block_samples = stretch.block_samples();
        let interval_samples = stretch.interval_samples();
        let split = stretch.split_computation();
        assert!(input_latency >= 0);
        assert!(output_latency >= 0);
        assert!(block_samples > 0);
        assert!(interval_samples > 0);
        assert!(!split);

        stretch.configure(2, 1024, 256, true);
        let split = stretch.split_computation();
        assert!(split);

        stretch.set_transpose_semitoses(2.0, Some(0.5));
        stretch.set_transpose_factor(1.5, Some(0.5));
        stretch.set_formant_factor(1.2, true);
        stretch.set_formant_semitones(-2.0, true);
        stretch.set_formant_base_frequency(500.0);

        let input = vec![0.0f32; 2048 * 2];
        let mut output = vec![0.0f32; 2048 * 2];
        stretch.seek(&input, 2048, 1.0);
        stretch.flush(&mut output, 2048, 1.0);

        let seek_length = stretch.seek_length();
        let output_seek_length = stretch.output_seek_length(1.0);

        assert!(seek_length >= 0);
        assert!(output_seek_length >= 0);

        stretch.output_seek(&input, 2048);
        stretch.process(&input, 2048, &mut output, 2048);
        
        stretch.reset();
        let input = vec![0.0f32; 44100 * 2];
        let mut output = vec![0.0f32; 44100 * 2];
        let exact_result = stretch.exact(&input, 44100, &mut output, 44100);
        assert!(exact_result);
    }
}