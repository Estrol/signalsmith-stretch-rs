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
    New(u32, usize, usize),
    PresetDefault(u32, u32),
    PresetCheaper(u32, u32),
}

/// Provides time stretching and/or pitch shifting for audio.
#[derive(Debug)]
pub struct Stretch {
    inner: *mut sys::signalsmith_stretch_t,
    channel_count: usize,
    init_type: InitType,
}

impl Clone for Stretch {
    fn clone(&self) -> Self {
        let ptr = unsafe {
            match self.init_type {
                InitType::New(channel_count, block_length, interval) => {
                    sys::signalsmith_stretch_create(channel_count as _, block_length, interval)
                }
                InitType::PresetDefault(channel_count, sample_rate) => {
                    sys::signalsmith_stretch_create_preset_default(channel_count as i32, sample_rate as f32)
                }
                InitType::PresetCheaper(channel_count, sample_rate) => {
                    sys::signalsmith_stretch_create_preset_cheaper(channel_count as i32, sample_rate as f32)
                }
            }
        };

        Stretch {
            inner: ptr,
            channel_count: self.channel_count,
            init_type: self.init_type.clone(),
        }
    }
}

unsafe impl Send for Stretch {}
unsafe impl Sync for Stretch {}

impl Stretch {
    /// Creates a new instance with a FFT window size and interval determined by
    /// `block_length`.
    pub fn new(channel_count: u32, block_length: usize, interval: usize) -> Self {
        let ptr =
            unsafe { sys::signalsmith_stretch_create(channel_count as _, block_length, interval) };

        Self {
            inner: ptr,
            channel_count: channel_count as usize,
            init_type: InitType::New(channel_count, block_length, interval),
        }
    }

    /// Creates a new instance with default values determined by the sample rate.
    pub fn preset_default(channel_count: u32, sample_rate: u32) -> Self {
        let ptr = unsafe {
            sys::signalsmith_stretch_create_preset_default(channel_count as i32, sample_rate as f32)
        };

        Self {
            inner: ptr,
            channel_count: channel_count as usize,
            init_type: InitType::PresetDefault(channel_count, sample_rate),
        }
    }

    /// Creates a new instance with default values determined by the sample rate,
    /// tweaked to be less computationally expensive.
    pub fn preset_cheaper(channel_count: u32, sample_rate: u32) -> Self {
        let ptr = unsafe {
            sys::signalsmith_stretch_create_preset_cheaper(channel_count as i32, sample_rate as f32)
        };

        Self {
            inner: ptr,
            channel_count: channel_count as usize,
            init_type: InitType::PresetCheaper(channel_count, sample_rate),
        }
    }

    /// Reset the instance to its initial (configured) state.
    pub fn reset(&mut self) {
        unsafe { sys::signalsmith_stretch_reset(self.inner) }
    }

    /// Get the current input latency, in frames. This is the delay between
    /// samples passed to process and the center of the any pitch-shift or
    /// stretch effect.
    pub fn input_latency(&self) -> usize {
        unsafe { sys::signalsmith_stretch_input_latency(self.inner) }
    }

    /// Get the current output latency, in frames. This is the delay between
    /// the center of any pitch/shift or stretch effect and the output generated
    /// by [Self::process].
    pub fn output_latency(&self) -> usize {
        unsafe { sys::signalsmith_stretch_output_latency(self.inner) }
    }

    /// Set the frequency multiplier and an optional tonality limit.
    pub fn set_transpose_factor(&mut self, multiplier: f32, tonality_limit: Option<f32>) {
        unsafe {
            sys::signalsmith_stretch_set_transpose_factor(
                self.inner,
                multiplier,
                tonality_limit.unwrap_or(0.0),
            )
        }
    }

    /// Set the frequency multiplier (in semitones) and an optional tonality
    /// limit.
    pub fn set_transpose_factor_semitones(&mut self, semitones: f32, tonality_limit: Option<f32>) {
        unsafe {
            sys::signalsmith_stretch_set_transpose_factor_semitones(
                self.inner,
                semitones,
                tonality_limit.unwrap_or(0.0),
            )
        }
    }

    /// Add "pre-roll" to the output without affecting the stream position.
    ///
    /// Input samples must be interleaved, with the correct number of channels.
    pub fn seek(&mut self, input: impl AsRef<[f32]>, playback_rate: f64) {
        let input = input.as_ref();
        let ptr = input.as_ptr();

        // Some sample rate like 44100 will have a frame count of 441, which is not divisible by common channel counts like 2 or 4.
        // debug_assert_eq!(0, input.len() % self.channel_count);

        unsafe {
            sys::signalsmith_stretch_seek(
                self.inner,
                ptr as _,
                input.len() / self.channel_count,
                playback_rate,
            );
        }
    }

    pub fn seek_raw(&mut self, input: impl AsRef<[f32]>, frame_count: usize, playback_rate: f64)
    {
        let input = input.as_ref();
        let ptr = input.as_ptr();

        // Some sample rate like 44100 will have a frame count of 441, which is not divisible by common channel counts like 2 or 4.
        // debug_assert_eq!(0, frame_count % self.channel_count);

        unsafe {
            sys::signalsmith_stretch_seek(
                self.inner,
                ptr as _,
                frame_count,
                playback_rate,
            );
        }
    }

    /// Add input to the stream, and get output. The length of input and output
    /// may differ, which will create a time-stretch effect.
    ///
    /// Input samples must be interleaved, with the correct number of channels.
    /// The output will be the same.
    pub fn process(&mut self, input: impl AsRef<[f32]>, mut output: impl AsMut<[f32]>) {
        let input = input.as_ref();
        let output = output.as_mut();

        // Some sample rate like 44100 will have a frame count of 441, which is not divisible by common channel counts like 2 or 4.
        // debug_assert_eq!(0, input.len() % self.channel_count);
        // debug_assert_eq!(0, output.len() % self.channel_count);

        unsafe {
            sys::signalsmith_stretch_process(
                self.inner,
                input.as_ptr() as _,
                input.len() / self.channel_count,
                output.as_mut_ptr(),
                output.len() / self.channel_count,
            );
        }
    }

    pub fn process_raw(&mut self, input: impl AsRef<[f32]>, frame_count: usize, mut output: impl AsMut<[f32]>, output_frame_count: usize) {
        let input = input.as_ref();
        let output = output.as_mut();

        // Some sample rate like 44100 will have a frame count of 441, which is not divisible by common channel counts like 2 or 4.
        // debug_assert_eq!(0, frame_count % self.channel_count);
        // debug_assert_eq!(0, output_frame_count % self.channel_count);

        unsafe {
            sys::signalsmith_stretch_process(
                self.inner,
                input.as_ptr() as _,
                frame_count,
                output.as_mut_ptr(),
                output_frame_count,
            );
        }
    }

    /// Flush remaining output from the decoder. Use [Self::output_latency] to
    /// determine the correct length of the output buffer.
    pub fn flush(&mut self, mut output: impl AsMut<[f32]>) {
        let output = output.as_mut();
        debug_assert_eq!(0, output.len() % self.channel_count);

        unsafe {
            sys::signalsmith_stretch_flush(
                self.inner,
                output.as_mut_ptr(),
                output.len() / self.channel_count,
            );
        }
    }

    pub fn flush_raw(&mut self, mut output: impl AsMut<[f32]>, frame_count: usize) {
        let output = output.as_mut();
        debug_assert_eq!(0, frame_count % self.channel_count);

        unsafe {
            sys::signalsmith_stretch_flush(
                self.inner,
                output.as_mut_ptr(),
                frame_count,
            );
        }
    }
}

impl Drop for Stretch {
    fn drop(&mut self) {
        unsafe { sys::signalsmith_stretch_destroy(self.inner) }
    }
}
