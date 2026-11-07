use anyhow::{Context, Result};
use log::LevelFilter::Debug;
use simplelog::*;
use std::fs::{File, create_dir_all};
use std::path::{Path, PathBuf};

/// Size above which the log file is emptied when the file logger is initialized.
const MAX_LOG_SIZE: u64 = 1024 * 1024;

#[cfg(not(test))]
static _LOG: &str = env!("LOG");
#[cfg(test)]
pub const _LOG: &str = "target/test/linux-enable-ir-emitter.log";

fn log_file_path() -> Result<String> {
    Ok(shellexpand::env(_LOG)
        .context("failed to expand shell variables in log file path")?
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
    let file = open_log_file(&path)?;

    let config = Config::default();
    WriteLogger::init(Debug, config, file).context("failed to initialize file logger")
}

/// Opens the log file in append mode, or empties it if it is bigger than [`MAX_LOG_SIZE`],
/// so it does not grow indefinitely.
fn open_log_file(path: &Path) -> Result<File> {
    let too_big = std::fs::metadata(path).is_ok_and(|m| m.len() > MAX_LOG_SIZE);
    File::options()
        .create(true)
        .write(true)
        .append(!too_big)
        .truncate(too_big)
        .open(path)
        .context("failed to open or create log file")
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
    use std::io::Write;

    #[test]
    fn test_open_log_file() {
        let dir = Path::new("target/test/open_log");
        std::fs::create_dir_all(dir).unwrap();
        let path = dir.join("test.log");
        let _ = std::fs::remove_file(&path);

        // missing file: created
        drop(open_log_file(&path).unwrap());
        assert_eq!(std::fs::metadata(&path).unwrap().len(), 0);

        // small file: appended
        std::fs::write(&path, "small").unwrap();
        write!(open_log_file(&path).unwrap(), "+").unwrap();
        assert_eq!(std::fs::read_to_string(&path).unwrap(), "small+");

        // big file: emptied
        std::fs::write(&path, vec![b'a'; MAX_LOG_SIZE as usize + 1]).unwrap();
        write!(open_log_file(&path).unwrap(), "+").unwrap();
        assert_eq!(std::fs::read_to_string(&path).unwrap(), "+");
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
