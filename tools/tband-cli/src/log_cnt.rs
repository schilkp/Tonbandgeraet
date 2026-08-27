use log::{Level, Log, Metadata, Record};
use std::sync::atomic::{AtomicUsize, Ordering};

// Tracks how many `warn!`/`error!` log records have been issued by tband.
// Used for `--fail-on-warn`.
static WARN_COUNT: AtomicUsize = AtomicUsize::new(0);

// Log target/module-path prefixes to consider:
const TBAND_LOG_TARGETS: &[&str] = &["tband_"];

fn is_tband_record(record: &Record) -> bool {
    TBAND_LOG_TARGETS
        .iter()
        .any(|prefix| record.target().starts_with(prefix))
}

pub struct CountingLogger {
    pub inner: env_logger::Logger,
}

impl Log for CountingLogger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        self.inner.enabled(metadata)
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) && record.level() <= Level::Warn && is_tband_record(record) {
            WARN_COUNT.fetch_add(1, Ordering::Relaxed);
        }
        self.inner.log(record);
    }

    fn flush(&self) {
        self.inner.flush();
    }
}

pub fn get_warn_count() -> usize {
    WARN_COUNT.load(Ordering::SeqCst)
}
