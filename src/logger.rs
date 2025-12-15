use anyhow::{Context, Result};
use log::LevelFilter::Debug;
use simplelog::*;
use std::fs::{File, create_dir_all};
use std::path::PathBuf;

#[cfg(not(test))]
static _LOG: &str = env!("LOG");
#[cfg(test)]
pub const _LOG: &str = "target/tmp/linux-enable-ir-emitter.log";

fn log_file_path() -> Result<String> {
    Ok(shellexpand::env(_LOG)
        .context("failed to expend shell variable in log file path")?
        .to_string())
}

pub fn init_term() -> Result<()> {
    let config = Config::default();
    TermLogger::init(Debug, config, TerminalMode::Mixed, ColorChoice::Auto)
        .context("failed to initialize terminal logger")
}

pub fn init_file() -> Result<()> {
    let path = PathBuf::from(log_file_path()?);
    if let Some(parent) = path.parent()
        && !parent.exists()
    {
        create_dir_all(parent).context("failed to create log directory")?;
    }
    let file = File::options()
        .create(true)
        .append(true)
        .open(path)
        .context("failed to open or create log file")?;

    let config = Config::default();
    WriteLogger::init(Debug, config, file).context("failed to initialize file logger")
}

pub fn print_log() -> Result<()> {
    let log_file = log_file_path()?;
    let content_str = std::fs::read_to_string(&log_file).context("failed to read log file")?;
    print!("# {}\n\n{}", log_file, content_str);
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_log_file() {
        // Ensure the log file is clean before starting the test
        let log_file = log_file_path().unwrap();
        let _ = std::fs::remove_file(&log_file);

        init_file().unwrap();
        log::trace!("This is a trace message");
        log::debug!("This is a debug message");
        log::info!("This is an info message");
        log::warn!("This is a warning message");
        log::error!("This is an error message");

        // Give some time for the logger to write to the file
        std::thread::sleep(std::time::Duration::from_millis(100));

        let content = std::fs::read_to_string(log_file).unwrap();
        assert!(!content.contains("This is a trace message"));
        assert!(content.contains("This is a debug message"));
        assert!(content.contains("This is an info message"));
        assert!(content.contains("This is a warning message"));
        assert!(content.contains("This is an error message"));
    }
}
