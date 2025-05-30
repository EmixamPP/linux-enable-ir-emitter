use crate::uvc::Control;

use std::collections::HashMap;
use std::path::{Path, PathBuf};

use anyhow::{Context, Result, anyhow};
use log::warn;
use serde::{Deserialize, Serialize};

const V4L_BY_ID_PATH: &str = "/dev/v4l/by-id/";
const CONFIG_PATH: &str = env!("CONFIG_PATH");

/// Represents a control that can be applied to a device.
#[derive(Debug, Deserialize, Serialize)]
pub struct WhiteControl {
    /// Unit used on the device to identify the control to apply.
    pub unit: u8,
    /// Unit used on the device to identify the control to apply.
    pub selector: u8,
    /// The value to apply to the control.
    pub value: Vec<u8>,
}

/// Represents a control that may break the device if applied.
#[derive(Debug, Deserialize, Serialize)]
pub struct BlackControl {
    /// Unit used on the device to identify the risky control.
    pub unit: u8,
    /// Selector used on the device to identify the risky control.
    pub selector: u8,
}

/// Represents a control configuration for a device.
#[derive(Debug, Deserialize, Serialize, Default)]
pub struct Config {
    /// The controls that have to be applied to the device.
    pub white_list: Vec<WhiteControl>,
    /// The controls that may break the device if applied.
    pub black_list: Vec<BlackControl>,
}

/// Represents the configuration for the device controls.
#[derive(Debug, Deserialize, Serialize, Default)]
struct Configs {
    /// A map of configuration files, where the key is the path to the device
    /// and the value is the control configuration for it.
    pub configs: HashMap<PathBuf, Config>,
}

impl Config {
    /// Find the equivalent the /dev/v4l/by-id/ path for better persistence.
    ///
    /// Indeed on some systems/hardware, the device path may change between reboots.
    fn by_id_device_path(device: &Path) -> Result<PathBuf> {
        let device_canonicalized = device.canonicalize()?;

        if let Ok(entries) = std::fs::read_dir(V4L_BY_ID_PATH) {
            for entry in entries.flatten() {
                let path = entry.path();
                if let Ok(target) = std::fs::read_link(&path) {
                    if let Ok(target_canonicalized) = target.canonicalize() {
                        if target_canonicalized == device_canonicalized {
                            return Ok(path);
                        }
                    }
                }
            }
        }

        Err(anyhow!(
            "No {V4L_BY_ID_PATH} path found for device: {device:?}"
        ))
    }

    /// Save the configuration for a device to the config file.
    pub fn save(self, device: &Path) -> Result<()> {
        let device_buf = Self::by_id_device_path(device).unwrap_or_else(|err| {
            warn!("{err}");
            device.to_path_buf()
        });

        // Load existing configs
        let mut configs = Configs::load_or_new();
        configs.configs.insert(device_buf, self);
        // Save the updated configs
        configs
            .save()
            .context(format! {"failed to save configuration to {}", CONFIG_PATH})
    }

    /// Loads the configuration for a device from the config file.
    /// Or creates a new one if it does not exist.
    pub fn load(device: &Path) -> Self {
        Configs::load()
            .and_then(|mut configs| configs.configs.remove(device).context("no config found"))
            .unwrap_or_default()
    }
}

impl Configs {
    /// Loads the configuration from the config file.
    ///
    /// # Errors
    /// Returns an anyhow error if the config file cannot be opened or parsed.
    pub fn load() -> Result<Self> {
        let file = std::fs::File::open(CONFIG_PATH)?;
        Ok(ron::de::from_reader(file)?)
    }

    /// Loads the configuration from the config file.
    /// Or creates a new one if it does not exist.
    pub fn load_or_new() -> Self {
        Self::load().unwrap_or_default()
    }

    pub fn save(&self) -> Result<()> {
        // Create the config directory if it does not exist
        let file = PathBuf::from(CONFIG_PATH);
        if let Some(dir) = file.parent() {
            if !dir.exists() {
                std::fs::create_dir_all(dir)?;
            }
        }

        // Save the config to the config file
        let file = std::fs::File::create(CONFIG_PATH).unwrap();
        ron::Options::default().to_io_writer_pretty(file, &self, ron::ser::PrettyConfig::new())?;

        Ok(())
    }
}

impl Control for WhiteControl {
    fn unit(&self) -> u8 {
        self.unit
    }

    fn selector(&self) -> u8 {
        self.selector
    }

    fn value_ref(&self) -> &[u8] {
        &self.value
    }

    fn value_mut(&mut self) -> &mut [u8] {
        &mut self.value
    }

    fn value(self) -> Vec<u8> {
        self.value
    }
}

impl From<&dyn Control> for WhiteControl {
    fn from(control: &dyn Control) -> Self {
        WhiteControl {
            unit: control.unit(),
            selector: control.selector(),
            value: control.value_ref().to_vec(),
        }
    }
}

impl From<&dyn Control> for BlackControl {
    fn from(control: &dyn Control) -> Self {
        BlackControl {
            unit: control.unit(),
            selector: control.selector(),
        }
    }
}
