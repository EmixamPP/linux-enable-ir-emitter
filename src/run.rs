use crate::configuration::{Configuration, Configurations};
use crate::device_id::DeviceId;
use crate::video::uvc::Device;

use anyhow::{Context, Result, bail};
use std::path::Path;

/// Run saved configurations on devices.
///
/// A specific device or already opened file descriptor can be targeted. Note: if a fd is provided, a device must also be provided.
///
/// If a configuration cannot be applied on a device, the other devices are still configured,
/// and an error is returned at the end.
///
/// Does not return an error if no configuration exists for the device.
pub fn run(device: Option<&Path>, fd: Option<i32>, config: Option<&Path>) -> Result<()> {
    if fd.is_some() && device.is_none() {
        bail!("if a file descriptor is provided, a device path must also be provided");
    }

    let configs = if let Some(config) = config {
        Configurations::load_from(config)?
    } else {
        Configurations::load()?
    };

    let selected = select_configurations(&configs, device);
    if let Some(device) = device
        && selected.is_empty()
    {
        log::warn!("No configuration found for device {}.", device.display());
    }

    let mut failures = 0;
    for (path, conf) in selected {
        if let Err(err) = apply_configuration(path, fd, conf) {
            log::error!("{err:#}");
            failures += 1;
        }
    }

    if failures > 0 {
        bail!("failed to apply {failures} configuration(s)");
    }
    Ok(())
}

/// Returns the configurations of the given device, or all of them if no device is specified.
///
/// The device is matched on its persistent identifier, so any path to it matches (e.g. `/dev/videoX` or a symlink).
fn select_configurations<'a>(
    configs: &'a Configurations,
    device: Option<&Path>,
) -> Vec<(&'a DeviceId, &'a Configuration)> {
    configs
        .devices()
        .iter()
        .filter(|(id, _)| device.is_none_or(|d| id.matches(d)))
        .collect()
}

/// Applies all the saved controls of a configuration on its device.
///
/// If a control cannot be applied, the remaining ones are still tried.
fn apply_configuration(id: &DeviceId, fd: Option<i32>, conf: &Configuration) -> Result<()> {
    let device = if let Some(fd) = fd {
        Device::from_fd(fd)?
    } else {
        let path = id
            .resolve()
            .with_context(|| format!("the device {id} is not connected"))?;
        if matches!(id, DeviceId::Usb(_)) {
            log::debug!("Device {id} is {}.", path.display());
        }
        Device::open(&path).with_context(|| format!("failed to open device {}", path.display()))?
    };

    let mut failures = 0;
    for mut control in conf.get_savelist() {
        if let Err(err) = device.apply_control(&mut control) {
            log::error!("Failed to apply control {control} on device {id}: {err}.");
            failures += 1;
        }
    }

    if failures > 0 {
        bail!("failed to apply {failures} control(s) on device {id}");
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Writes a configuration file with two non-existing devices and loads it.
    fn make_configs(name: &str) -> (std::path::PathBuf, Configurations) {
        let dir = Path::new("target/tmp/run");
        std::fs::create_dir_all(dir).unwrap();
        let file = dir.join(name);
        std::fs::write(
            &file,
            r#"
[["/dev/video_run_test_a".savelist]]
unit = 1
selector = 1
control = [1]

[["/dev/video_run_test_b".savelist]]
unit = 2
selector = 2
control = [2]
"#,
        )
        .unwrap();
        let configs = Configurations::load_from(&file).unwrap();
        (file, configs)
    }

    #[test]
    fn test_select_all_configurations() {
        let (_, configs) = make_configs("select_all.toml");
        assert_eq!(select_configurations(&configs, None).len(), 2);
    }

    #[test]
    fn test_select_one_configuration() {
        let (_, configs) = make_configs("select_one.toml");
        let selected = select_configurations(&configs, Some(Path::new("/dev/video_run_test_b")));
        assert_eq!(selected.len(), 1);
        assert_eq!(selected[0].0.to_string(), "/dev/video_run_test_b");
    }

    #[test]
    fn test_select_unknown_device() {
        let (_, configs) = make_configs("select_unknown.toml");
        assert!(select_configurations(&configs, Some(Path::new("/dev/video_unknown"))).is_empty());
    }

    #[test]
    fn test_run_continues_after_a_failure() {
        let (file, _) = make_configs("continue.toml");
        let err = run(None, None, Some(&file)).unwrap_err();
        // both devices do not exist, so both must have been tried
        assert_eq!(err.to_string(), "failed to apply 2 configuration(s)");
    }

    #[test]
    fn test_run_unknown_device_is_ok() {
        let (file, _) = make_configs("unknown_ok.toml");
        assert!(run(Some(Path::new("/dev/video_unknown")), None, Some(&file)).is_ok());
    }

    #[test]
    fn test_run_fd_requires_device() {
        assert!(run(None, Some(3), None).is_err());
    }
}
