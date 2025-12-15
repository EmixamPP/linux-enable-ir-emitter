pub mod ir;
pub mod stream;
pub mod uvc;

use anyhow::{Result, bail};

pub fn print_grey_devices() -> Result<()> {
    let devices = stream::grey_devices();
    if devices.is_empty() {
        bail!("No grey scale video devices found.");
    }

    for path in devices {
        let device = uvc::Device::open(path)?;
        print!("{}", device);
    }
    Ok(())
}
