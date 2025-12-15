use crate::video::uvc::XuControl;

use std::collections::{BTreeMap as Map, BTreeSet as Set};
use std::path::Path;

use anyhow::{Context, Result, anyhow};
use derive_more::AsRef;
use serde::{Deserialize, Serialize};

const V4L_DEV_DIR: &str = "/dev/v4l/by-id";
#[cfg(not(test))]
static _CONFIG: &str = env!("CONFIG");
#[cfg(test)]
pub const _CONFIG: &str = "target/tmp/linux-enable-ir-emitter.toml";

fn config_file_path() -> Result<String> {
    Ok(shellexpand::env(_CONFIG)
        .context("failed to expend shell variable in log file path")?
        .to_string())
}

pub fn print_config() -> Result<()> {
    let config_file = config_file_path()?;
    let content_str =
        std::fs::read_to_string(&config_file).context("failed to read configuration file")?;
    print!("# {}\n\n{}", config_file, content_str);
    Ok(())
}

mod path {
    use super::*;
    use std::path::PathBuf;

    #[derive(Default, Debug, PartialEq, Eq, PartialOrd, Ord, Clone, AsRef)]
    pub struct V4LPath(PathBuf);

    impl V4LPath {
        /// Create a new V4LPath, attempting to find a persistent path in /dev/v4l/by-id
        /// If not found, use the provided path as is.
        pub fn new<P: AsRef<Path>>(device: P) -> Self {
            Self(Self::find_v4l_path(device.as_ref()).unwrap_or(device.as_ref().to_path_buf()))
        }

        fn find_v4l_path(device: &Path) -> Option<PathBuf> {
            if let Ok(entries) = std::fs::read_dir(Path::new(V4L_DEV_DIR)) {
                for entry in entries.filter_map(Result::ok) {
                    if let Ok(target) = entry.path().canonicalize()
                        && target == device
                    {
                        return Some(entry.path());
                    }
                }
            }
            None
        }
    }

    impl Serialize for V4LPath {
        fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where
            S: serde::Serializer,
        {
            serializer.serialize_str(&self.0.to_string_lossy())
        }
    }

    impl<'de> Deserialize<'de> for V4LPath {
        fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
        where
            D: serde::Deserializer<'de>,
        {
            Ok(Self(Deserialize::deserialize(deserializer)?))
        }
    }
}

#[derive(Clone, Serialize, Deserialize, Debug, Ord, PartialOrd, Eq, PartialEq)]
struct SavedControl {
    unit: u8,
    selector: u8,
    control: Vec<u8>,
}

impl From<&XuControl> for SavedControl {
    fn from(control: &XuControl) -> Self {
        Self {
            unit: control.unit(),
            selector: control.selector(),
            control: control.cur().to_vec(),
        }
    }
}

#[derive(Clone, Serialize, Deserialize, Debug, Ord, PartialOrd, Eq, PartialEq)]
struct BlackControl {
    unit: u8,
    selector: u8,
}

impl From<&XuControl> for BlackControl {
    fn from(control: &XuControl) -> Self {
        Self {
            unit: control.unit(),
            selector: control.selector(),
        }
    }
}

#[derive(Default, Debug)]
pub struct Configurations {
    devices: Map<path::V4LPath, Configuration>,
}

impl Configurations {
    /// Saves the configurations.
    pub fn save(&self) -> Result<()> {
        Self::initialize_config_dir()?;
        let config_str =
            toml::to_string(&self.devices).context("failed to serialize configuration")?;
        let config_file = config_file_path()?;
        log::debug!("Saving configuration at {}:\n{}", config_file, config_str);
        std::fs::write(config_file, config_str).context("failed to write configuration file")
    }

    /// Load an existing configurations or create an empty one.
    pub fn load() -> Result<Self> {
        Ok(match std::fs::read_to_string(config_file_path()?) {
            Ok(config_str) => Self {
                devices: toml::from_str(&config_str)
                    .context("failed to parse configuration file")?,
            },
            Err(_) => Self::default(),
        })
    }

    pub fn devices(&self) -> &Map<path::V4LPath, Configuration> {
        &self.devices
    }

    /// Initializes an existing configuration if it does not exist.
    fn initialize_config_dir() -> Result<()> {
        let config_file = config_file_path()?;
        log::debug!("Configuration located at: {}", config_file);
        let config_dir = Path::new(&config_file).parent();
        if let Some(config_dir) = config_dir {
            std::fs::create_dir_all(config_dir).context("failed to create configuration directory")
        } else {
            Ok(())
        }
    }
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct Configuration {
    #[serde(skip_serializing_if = "Set::is_empty", default)]
    savelist: Set<SavedControl>,
    #[serde(skip_serializing_if = "Set::is_empty", default)]
    blacklist: Set<BlackControl>,
    #[serde(skip)]
    device: path::V4LPath,
}

impl Configuration {
    /// Creates a new configuration for the specified device.
    /// If a configuration was saved previously, it is loaded.
    pub fn new<P: AsRef<Path>>(device: P) -> Self {
        let device = path::V4LPath::new(device);

        match Self::load(&device) {
            Ok(config) => config,
            Err(err) => {
                log::debug!("No existing configuration found: {err:?}.");
                Self {
                    savelist: Set::new(),
                    blacklist: Set::new(),
                    device,
                }
            }
        }
    }

    /// Adds a control to the savelist.
    pub fn add_to_savelist(&mut self, control: &XuControl) {
        self.savelist.insert(control.into());
    }

    /// Adds a control to the blacklist.
    pub fn add_to_blacklist(&mut self, control: &XuControl) {
        self.blacklist.insert(control.into());
    }

    /// Returns true if the control is blacklisted.
    pub fn is_blacklisted(&self, control: &XuControl) -> bool {
        self.blacklist
            .iter()
            .any(|bc| bc.unit == control.unit() && bc.selector == control.selector())
    }

    /// Returns the list of saved controls.
    pub fn get_savelist(&self) -> Vec<XuControl> {
        self.savelist
            .iter()
            .map(|wc| {
                XuControl::new(
                    wc.unit,
                    wc.selector,
                    wc.control.clone(),
                    None,
                    None,
                    None,
                    None,
                    true,
                )
                .context("please report this as a bug: invalid XuControl in savelist")
                .unwrap()
            })
            .collect()
    }

    fn load(device: &path::V4LPath) -> Result<Self> {
        let configs = Configurations::load()?;
        if let Some(config) = configs.devices().get(device) {
            let mut config = config.clone();
            config.device = device.clone();
            Ok(config)
        } else {
            Err(anyhow!(
                "no configuration found for device {}",
                device.as_ref().display()
            ))
        }
    }

    pub fn save(&self) -> Result<()> {
        let mut configs = Configurations::load().unwrap_or_default();

        // do not serialize empty configuration
        if self.savelist.is_empty() && self.blacklist.is_empty() {
            let _ = configs.devices.remove(&self.device);
        } else {
            let _ = configs.devices.insert(self.device.clone(), self.clone());
        }

        configs.save()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use rand::prelude::*;
    use rand_chacha;
    use serial_test::serial;
    use std::path::PathBuf;

    fn make_xu(rng: &mut rand_chacha::ChaCha8Rng) -> XuControl {
        let unit = rng.random_range(1..=255);
        let selector = rng.random_range(1..=255);
        let len = rng.random_range(1..=12);
        let current: Vec<u8> = (0..len).map(|_| rng.random()).collect();
        XuControl::new(unit, selector, current, None, None, None, None, true).unwrap()
    }

    /// Make sure the config dir is deleted and isolated for each test.
    fn clean_config_dir() {
        let _ = std::fs::remove_dir_all(Path::new(&config_file_path().unwrap()).parent().unwrap());
    }

    // Make a RNG with a fixed seed for reproducible tests.
    fn make_rng() -> rand_chacha::ChaCha8Rng {
        rand::SeedableRng::seed_from_u64(666)
    }

    #[test]
    fn test_v4lpath_new_returns_given_path_if_not_found() {
        let path = PathBuf::from("/dev/video10");
        let v4l_path = path::V4LPath::new("/dev/video10");
        assert_eq!(v4l_path.as_ref(), &path);
    }

    #[test]
    fn test_whitecontrol_from_xucontrol() {
        let xu = make_xu(&mut make_rng());
        let wc = SavedControl::from(&xu);
        assert_eq!(wc.unit, xu.unit());
        assert_eq!(wc.selector, xu.selector());
        assert_eq!(wc.control, xu.cur());
    }

    #[test]
    fn test_blackcontrol_from_xucontrol() {
        let xu = make_xu(&mut make_rng());
        let bc = BlackControl::from(&xu);
        assert_eq!(bc.unit, xu.unit());
        assert_eq!(bc.selector, xu.selector());
    }

    #[test]
    #[serial]
    fn test_configuration_add_and_check_blacklist() {
        clean_config_dir();
        let xu = make_xu(&mut make_rng());
        let mut config = Configuration::new("/dev/video0");
        assert!(!config.is_blacklisted(&xu));
        config.add_to_blacklist(&xu);
        assert!(config.is_blacklisted(&xu));
    }

    #[test]
    #[serial]
    fn test_configuration_add_to_savelist_and_get() {
        clean_config_dir();
        let xu = make_xu(&mut make_rng());
        let mut config = Configuration::new("/dev/video0");
        config.add_to_savelist(&xu);
        let list = config.get_savelist();
        assert_eq!(list.len(), 1);
        assert_eq!(list[0].unit(), xu.unit());
        assert_eq!(list[0].selector(), xu.selector());
        assert_eq!(list[0].cur(), xu.cur());
    }

    #[test]
    #[serial]
    fn test_initialize_config_path() {
        clean_config_dir();
        Configurations::initialize_config_dir().unwrap();
        assert!(
            Path::new(&config_file_path().unwrap())
                .parent()
                .unwrap()
                .exists()
        );
    }

    #[test]
    #[serial]
    fn test_configuration_empty_save_and_load() {
        clean_config_dir();
        let config = Configuration::new("/dev/video0");
        config.save().unwrap();
        assert!(Configuration::load(&config.device).is_err());
    }

    #[test]
    #[serial]
    fn test_configuration_empty_blacklist_save_and_load() {
        clean_config_dir();
        let mut rng = make_rng();

        let mut config = Configuration::new("/dev/video0");
        let xu1 = make_xu(&mut rng);
        config.add_to_savelist(&xu1);
        config.save().unwrap();

        let loaded = Configuration::load(&config.device).unwrap();
        assert_eq!(loaded.device.as_ref(), config.device.as_ref());
        assert_eq!(loaded.savelist.len(), 1);
        let saved = loaded.savelist.iter().next().unwrap();
        assert_eq!(saved.unit, xu1.unit());
        assert_eq!(saved.selector, xu1.selector());
        assert_eq!(saved.control, xu1.cur());
        assert_eq!(loaded.blacklist.len(), 0);
    }

    #[test]
    #[serial]
    fn test_configuration_empty_savelist_save_and_load() {
        clean_config_dir();
        let mut rng = make_rng();

        let mut config = Configuration::new("/dev/video0");
        let xu1 = make_xu(&mut rng);
        config.add_to_blacklist(&xu1);
        config.save().unwrap();

        let loaded = Configuration::load(&config.device).unwrap();
        assert_eq!(loaded.device.as_ref(), config.device.as_ref());
        assert_eq!(loaded.blacklist.len(), 1);
        let black = loaded.blacklist.iter().next().unwrap();
        assert_eq!(black.unit, xu1.unit());
        assert_eq!(black.selector, xu1.selector());
        assert_eq!(loaded.savelist.len(), 0);
    }

    #[test]
    #[serial]
    fn test_configuration_save_and_load() {
        clean_config_dir();
        let mut rng = make_rng();

        let mut config = Configuration::new("/dev/video0");
        let xu1 = make_xu(&mut rng);
        let xu2 = make_xu(&mut rng);
        let xu3 = make_xu(&mut rng);
        let xu4 = make_xu(&mut rng);
        config.add_to_savelist(&xu1);
        config.add_to_savelist(&xu2);
        config.add_to_blacklist(&xu3);
        config.add_to_blacklist(&xu4);
        config.save().unwrap();

        let loaded = Configuration::load(&config.device).unwrap();
        assert_eq!(loaded.device.as_ref(), config.device.as_ref());
        assert_eq!(loaded.savelist.len(), 2);
        let saved1 = loaded
            .savelist
            .iter()
            .find(|c| c.unit == xu1.unit())
            .unwrap();
        assert_eq!(saved1.selector, xu1.selector());
        assert_eq!(saved1.control, xu1.cur());
        let saved2 = loaded
            .savelist
            .iter()
            .find(|c| c.unit == xu2.unit())
            .unwrap();
        assert_eq!(saved2.selector, xu2.selector());
        assert_eq!(saved2.control, xu2.cur());
        assert_eq!(loaded.blacklist.len(), 2);
        let black1 = loaded
            .blacklist
            .iter()
            .find(|c| c.unit == xu3.unit())
            .unwrap();
        assert_eq!(black1.selector, xu3.selector());
        let black2 = loaded
            .blacklist
            .iter()
            .find(|c| c.unit == xu4.unit())
            .unwrap();
        assert_eq!(black2.selector, xu4.selector());
    }

    #[test]
    #[serial]
    fn test_configuration_save_twice_and_load() {
        clean_config_dir();
        let mut rng = make_rng();

        let mut config = Configuration::new("/dev/video0");
        let xu1 = make_xu(&mut rng);
        let xu2 = make_xu(&mut rng);
        config.add_to_savelist(&xu1);
        config.add_to_blacklist(&xu2);
        config.save().unwrap();
        config.save().unwrap();

        let loaded = Configuration::load(&config.device).unwrap();
        assert_eq!(loaded.device.as_ref(), config.device.as_ref());
        assert_eq!(loaded.savelist.len(), 1);
        let saved = loaded.savelist.iter().next().unwrap();
        assert_eq!(saved.unit, xu1.unit());
        assert_eq!(saved.selector, xu1.selector());
        assert_eq!(saved.control, xu1.cur());
        assert_eq!(loaded.blacklist.len(), 1);
        let black = loaded.blacklist.iter().next().unwrap();
        assert_eq!(black.unit, xu2.unit());
        assert_eq!(black.selector, xu2.selector());
    }

    #[test]
    #[serial]
    fn test_configuration_save_duplicated_and_load() {
        clean_config_dir();
        let mut rng = make_rng();

        let mut config = Configuration::new("/dev/video0");
        let xu1 = make_xu(&mut rng);
        let xu2 = make_xu(&mut rng);
        config.add_to_savelist(&xu1);
        config.add_to_savelist(&xu1);
        config.add_to_blacklist(&xu2);
        config.add_to_blacklist(&xu2);
        config.save().unwrap();

        let loaded = Configuration::load(&config.device).unwrap();
        assert_eq!(loaded.device.as_ref(), config.device.as_ref());
        assert_eq!(loaded.savelist.len(), 1);
        let saved = loaded.savelist.iter().next().unwrap();
        assert_eq!(saved.unit, xu1.unit());
        assert_eq!(saved.selector, xu1.selector());
        assert_eq!(saved.control, xu1.cur());
        assert_eq!(loaded.blacklist.len(), 1);
        let black = loaded.blacklist.iter().next().unwrap();
        assert_eq!(black.unit, xu2.unit());
        assert_eq!(black.selector, xu2.selector());
    }

    #[test]
    #[serial]
    fn test_configuration_save_overwrite_save() {
        clean_config_dir();
        let mut rng = make_rng();

        let mut config = Configuration::new("/dev/video0");
        let xu1 = make_xu(&mut rng);
        let xu2 = make_xu(&mut rng);
        config.add_to_savelist(&xu1);
        config.add_to_blacklist(&xu2);
        config.save().unwrap();

        let mut config = Configuration::new("/dev/video0");
        // need to clear since it is loaded if already existing
        config.savelist.clear();
        config.blacklist.clear();
        let xu3 = make_xu(&mut rng);
        let xu4 = make_xu(&mut rng);
        config.add_to_savelist(&xu3);
        config.add_to_blacklist(&xu4);
        config.save().unwrap();

        let loaded = Configuration::load(&config.device).unwrap();
        assert_eq!(loaded.device.as_ref(), config.device.as_ref());
        assert_eq!(loaded.savelist.len(), 1);
        let saved = loaded.savelist.iter().next().unwrap();
        assert_eq!(saved.unit, xu3.unit());
        assert_eq!(saved.selector, xu3.selector());
        assert_eq!(saved.control, xu3.cur());
        assert_eq!(loaded.blacklist.len(), 1);
        let black = loaded.blacklist.iter().next().unwrap();
        assert_eq!(black.unit, xu4.unit());
        assert_eq!(black.selector, xu4.selector());
    }

    #[test]
    #[serial]
    fn test_configuration_save_clear_save() {
        clean_config_dir();
        let mut rng = make_rng();

        let mut config = Configuration::new("/dev/video0");
        let xu1 = make_xu(&mut rng);
        let xu2 = make_xu(&mut rng);
        config.add_to_savelist(&xu1);
        config.add_to_blacklist(&xu2);
        config.save().unwrap();

        let mut config = Configuration::new("/dev/video0");
        config.savelist.clear();
        config.blacklist.clear();
        config.save().unwrap();

        assert!(Configuration::load(&config.device).is_err());
    }
}
