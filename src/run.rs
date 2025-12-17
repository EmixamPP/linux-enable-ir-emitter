use crate::configuration::Configurations;
use crate::video::uvc::Device;

use anyhow::{Result, bail};
use std::path::Path;

/// Run saved configurations on devices.
///
/// A specific device or already opened file descriptor can be targeted. Note: if a fd is provided, a device must also be provided.
///
/// Does not return an error if no configuration exists for the device.
///
pub fn run(device: Option<&Path>, fd: Option<i32>, config: Option<&Path>) -> Result<()> {
    if fd.is_some() && device.is_none() {
        bail!("if a file descriptor is provided, a device path must also be provided");
    }

    let config = if let Some(config) = config {
        Configurations::load_from(config)?
    } else {
        Configurations::load()?
    };

    for (path, conf) in config.devices() {
        if let Some(d) = device
            && d != path.as_ref()
        {
            continue;
        }

        let device = if let Some(fd) = fd {
            Device::from_fd(fd)?
        } else {
            Device::open(path.as_ref())?
        };

        for mut control in conf.get_savelist() {
            device.apply_control(&mut control)?;
        }
    }
    Ok(())
}
