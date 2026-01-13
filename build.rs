use std::env;
use std::path::Path;

fn main() {
    let src_path = Path::new("src");

    println!("cargo::rerun-if-changed=src/stretch.h");
    println!("cargo::rerun-if-changed=src/stretch.cpp");
    println!("cargo::rerun-if-changed=src/fft.cpp");
    println!("cargo::rerun-if-changed=src/fft.h");
    println!("cargo::rerun-if-changed=src/stft.h");
    println!("cargo::rerun-if-changed=src/stft.cpp");
    println!("cargo::rerun-if-changed=src/signalsmith-stretch/signalsmith-stretch.h");
    println!("cargo::rerun-if-changed=src/signalsmith-stretch/linear.h");
    println!("cargo::rerun-if-changed=src/signalsmith-stretch/fft.h");
    println!("cargo::rerun-if-changed=src/signalsmith-stretch/stft.h");

    cc::Build::new()
        .file(src_path.join("stretch.cpp"))
        .file(src_path.join("fft.cpp"))
        .file(src_path.join("stft.cpp"))
        .flag_if_supported("-O3")
        .flag_if_supported("-ffast-math")
        .cpp(true)
        .compile("signalsmith-stretch");

    let bindings = bindgen::Builder::default()
        .header(src_path.join("stretch.h").as_os_str().to_str().unwrap())
        .header(src_path.join("fft.h").as_os_str().to_str().unwrap())
        .header(src_path.join("stft.h").as_os_str().to_str().unwrap())
        .generate()
        .expect("Unable to generate bindings");

    let out_path = Path::new(&env::var("OUT_DIR").unwrap()).join("bindings.rs");
    bindings
        .write_to_file(out_path)
        .expect("Couldn't write bindings!");
}
