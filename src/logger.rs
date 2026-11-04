use anyhow::{Context, Result};
use log::LevelFilter::Debug;
use simplelog::*;
use std::fs::{File, create_dir_all};
use std::path::{Path, PathBuf};

/// Size above which the log file is rotated when the file logger is initialized.
const MAX_LOG_SIZE: u64 = 1024 * 1024;

#[cfg(not(test))]
static _LOG: &str = env!("LOG");
#[cfg(test)]
pub const _LOG: &str = "target/test/linux-enable-ir-emitter.log";

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
    rotate_if_too_big(&path)?;
    let file = File::options()
        .create(true)
        .append(true)
        .open(path)
        .context("failed to open or create log file")?;

    let config = Config::default();
    WriteLogger::init(Debug, config, file).context("failed to initialize file logger")
}

/// Renames the log file to `<name>.old`, replacing the previous one, if it is bigger than [`MAX_LOG_SIZE`].
/// So the log file does not grow indefinitely, while keeping the logs of the previous sessions.
fn rotate_if_too_big(path: &Path) -> Result<()> {
    if let Ok(metadata) = std::fs::metadata(path)
        && metadata.len() > MAX_LOG_SIZE
    {
        let mut old_path = path.as_os_str().to_owned();
        old_path.push(".old");
        std::fs::rename(path, old_path).context("failed to rotate the log file")?;
    }
    Ok(())
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
    fn test_rotate_if_too_big() {
        let dir = Path::new("target/test/rotate");
        std::fs::create_dir_all(dir).unwrap();
        let path = dir.join("test.log");
        let old_path = dir.join("test.log.old");
        let _ = std::fs::remove_file(&old_path);

        // small file: kept
        std::fs::write(&path, "small").unwrap();
        rotate_if_too_big(&path).unwrap();
        assert!(path.exists());
        assert!(!old_path.exists());

        // big file: rotated
        std::fs::write(&path, vec![b'a'; MAX_LOG_SIZE as usize + 1]).unwrap();
        rotate_if_too_big(&path).unwrap();
        assert!(!path.exists());
        assert!(old_path.exists());

        // missing file: nothing to do
        rotate_if_too_big(&path).unwrap();
    }

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
