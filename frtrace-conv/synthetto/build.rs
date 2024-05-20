fn main() {
    let mut prost = prost_build::Config::new();
    if let Err(e) = prost.compile_protos(&["src/perfetto_trace_minimal.proto"], &["perfetto"]) {
        eprintln!("PROST proto compile error:");
        for line in e.to_string().lines() {
            eprintln!("{line}");
        }
        panic!();
    }
}
