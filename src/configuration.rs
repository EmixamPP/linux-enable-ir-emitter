use crate::device_id::DeviceId;
use crate::video::uvc::XuControl;

use std::collections::{BTreeMap as Map, BTreeSet as Set};
use std::path::{Path, PathBuf};

use anyhow::{Context, Result};
use serde::{Deserialize, Serialize};

#[cfg(not(test))]
static _CONFIG: &str = env!("CONFIG");
#[cfg(test)]
pub const _CONFIG: &str = "target/tmp/linux-enable-ir-emitter.toml";

fn config_file_path() -> Result<PathBuf> {
    Ok(shellexpand::env(_CONFIG)
        .context("failed to expand shell variables in configuration file path")?
        .to_string()
        .into())
}

/// A user for whom the tool is executed by PAM, e.g. through `pam_exec`.
#[derive(Debug, PartialEq, Eq)]
pub struct PamUser {
    name: String,
    uid: u32,
    home: PathBuf,
}

impl PamUser {
    /// Returns the user being authenticated if the tool is executed by PAM,
    /// i.e. if the `PAM_USER` and `PAM_TYPE` environment variables are set by `pam_exec`.
    pub fn from_env() -> Result<Option<Self>> {
        let (Ok(name), Ok(_)) = (std::env::var("PAM_USER"), std::env::var("PAM_TYPE")) else {
            return Ok(None);
        };
        let output = std::process::Command::new("getent")
            .args(["passwd", &name])
            .output()
            .context("failed to execute getent to find the user home directory")?;
        if !output.status.success() {
            anyhow::bail!("unknown user {name:?}");
        }
        Self::from_passwd(&String::from_utf8_lossy(&output.stdout)).map(Some)
    }

    /// Parses a passwd entry, e.g. `name:x:1000:1000:Full Name:/home/name:/bin/bash`.
    fn from_passwd(entry: &str) -> Result<Self> {
        let fields: Vec<&str> = entry.trim_end().split(':').collect();
        if fields.len() < 7 {
            anyhow::bail!("invalid passwd entry {entry:?}");
        }
        Ok(Self {
            name: fields[0].to_string(),
            uid: fields[2].parse().context("invalid uid in passwd entry")?,
            home: PathBuf::from(fields[5]),
        })
    }

    pub fn name(&self) -> &str {
        &self.name
    }

    /// Returns the configuration file path of the user, i.e. the default path expanded with their home directory.
    fn config_file_path(&self) -> PathBuf {
        let home = self.home.to_string_lossy();
        shellexpand::env_with_context_no_errors(_CONFIG, |var| match var {
            "HOME" => Some(home.to_string()),
            "USER" => Some(self.name.clone()),
            _ => std::env::var(var).ok(),
        })
        .to_string()
        .into()
    }
}

pub fn print_config() -> Result<()> {
    let config_file = config_file_path()?;
    let content_str =
        std::fs::read_to_string(&config_file).context("failed to read configuration file")?;
    print!("# {}\n\n{}", config_file.display(), content_str);
    Ok(())
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
    devices: Map<DeviceId, Configuration>,
}

impl Configurations {
    /// Saves the configurations.
    pub fn save(&self) -> Result<()> {
        Self::initialize_config_dir()?;
        let config_str =
            toml::to_string(&self.devices).context("failed to serialize configuration")?;
        let config_file = config_file_path()?;
        log::debug!(
            "Saving configuration at {}:\n{}",
            config_file.display(),
            config_str
        );
        std::fs::write(config_file, config_str).context("failed to write configuration file")
    }

    /// Load an existing configurations at the default location or create an empty one if there is none.
    ///
    /// Returns an error if the configuration file exists but cannot be read or parsed,
    /// so that it is never overwritten by mistake.
    pub fn load() -> Result<Self> {
        let config_file = config_file_path()?;
        if !config_file.exists() {
            log::debug!("No configuration file at {}.", config_file.display());
            return Ok(Self::default());
        }
        Self::load_from(&config_file)
    }

    /// Load the configurations of a user for whom the tool is executed by PAM, or an empty one if there is none.
    ///
    /// Since PAM executes the tool as root, the file must be a regular file owned by the user or root,
    /// so a user cannot make root read another file.
    pub fn load_for(user: &PamUser) -> Result<Self> {
        let config_file = user.config_file_path();
        let metadata = match std::fs::symlink_metadata(&config_file) {
            Ok(metadata) => metadata,
            Err(err) if err.kind() == std::io::ErrorKind::NotFound => {
                log::debug!("No configuration file at {}.", config_file.display());
                return Ok(Self::default());
            }
            Err(err) => {
                return Err(err).with_context(|| {
                    format!(
                        "failed to read configuration file {}",
                        config_file.display()
                    )
                });
            }
        };
        use std::os::unix::fs::MetadataExt as _;
        if !metadata.is_file() || (metadata.uid() != user.uid && metadata.uid() != 0) {
            anyhow::bail!(
                "refusing to read {}: it must be a regular file owned by {} or root",
                config_file.display(),
                user.name
            );
        }
        Self::load_from(&config_file)
    }

    /// Load an existing configurations at a specified path.
    pub fn load_from(config: &Path) -> Result<Self> {
        log::info!("Loading configuration from {}.", config.display());
        let config_str = std::fs::read_to_string(config)
            .with_context(|| format!("failed to read configuration file {}", config.display()))?;
        Ok(Self {
            devices: toml::from_str(&config_str).context("failed to parse configuration file")?,
        })
    }

    pub fn devices(&self) -> &Map<DeviceId, Configuration> {
        &self.devices
    }

    /// Initializes an existing configuration if it does not exist.
    fn initialize_config_dir() -> Result<()> {
        let config_file = config_file_path()?;
        log::debug!("Configuration located at: {}", config_file.display());
        if let Some(config_dir) = config_file.parent() {
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
    device: DeviceId,
    /// Configurations of the same device saved under a legacy path, replaced by this one when saving.
    #[serde(skip)]
    replaces: Vec<DeviceId>,
}

impl Configuration {
    /// Creates a new configuration for the specified device.
    /// If a configuration was saved previously, it is loaded.
    ///
    /// The configurations of the same USB device saved under a legacy path (e.g. `/dev/v4l/by-id/...`)
    /// are merged into it, and replaced by it when saving.
    pub fn new<P: AsRef<Path>>(device: P) -> Self {
        let device = DeviceId::new(device.as_ref());
        let configs = match Configurations::load() {
            Ok(configs) => configs,
            Err(err) => {
                log::debug!("No existing configuration loaded: {err:?}.");
                return Self::empty(device);
            }
        };

        let mut config = match configs.devices().get(&device) {
            Some(config) => Self {
                device: device.clone(),
                ..config.clone()
            },
            None => Self::empty(device.clone()),
        };
        let current = config.device.clone();
        config.adopt_legacy(&configs, |legacy| current.same_usb_device(legacy));
        config
    }

    fn empty(device: DeviceId) -> Self {
        Self {
            savelist: Set::new(),
            blacklist: Set::new(),
            device,
            replaces: Vec::new(),
        }
    }

    /// Merges the configurations saved under a legacy path for the same USB device,
    /// as told by `same_device`. The controls already in this configuration are kept.
    fn adopt_legacy(&mut self, configs: &Configurations, same_device: impl Fn(&DeviceId) -> bool) {
        if !matches!(self.device, DeviceId::Usb(_)) {
            return;
        }
        for (id, legacy) in configs.devices() {
            if !matches!(id, DeviceId::Path(_)) || !same_device(id) {
                continue;
            }
            log::info!(
                "Replacing the legacy configuration of {id} by {}.",
                self.device
            );
            for saved in &legacy.savelist {
                if !self
                    .savelist
                    .iter()
                    .any(|s| s.unit == saved.unit && s.selector == saved.selector)
                {
                    self.savelist.insert(saved.clone());
                }
            }
            self.blacklist.extend(legacy.blacklist.iter().cloned());
            self.replaces.push(id.clone());
        }
    }

    /// Adds a control to the savelist, replacing any previously saved value of this control.
    pub fn add_to_savelist(&mut self, control: &XuControl) {
        self.savelist
            .retain(|sc| sc.unit != control.unit() || sc.selector != control.selector());
        self.savelist.insert(control.into());
    }

    /// Adds a control to the blacklist.
    pub fn add_to_blacklist(&mut self, control: &XuControl) {
        self.blacklist.insert(control.into());
    }

    /// Returns true if no control is saved nor blacklisted.
    pub fn is_empty(&self) -> bool {
        self.savelist.is_empty() && self.blacklist.is_empty()
    }

    /// Returns the persistent identifier of the device of this configuration.
    pub fn device(&self) -> &DeviceId {
        &self.device
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

    #[cfg(test)]
    fn load(device: &DeviceId) -> Result<Self> {
        let configs = Configurations::load()?;
        if let Some(config) = configs.devices().get(device) {
            let mut config = config.clone();
            config.device = device.clone();
            Ok(config)
        } else {
            Err(anyhow::anyhow!(
                "no configuration found for device {device}"
            ))
        }
    }

    /// Saves this configuration, keeping the configurations of the other devices.
    ///
    /// Returns an error, without writing anything, if the existing configuration file cannot be read.
    pub fn save(&self) -> Result<()> {
        let mut configs = Configurations::load().context(
            "the existing configuration file cannot be read, please fix or remove it before saving",
        )?;

        for legacy in &self.replaces {
            let _ = configs.devices.remove(legacy);
        }

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
    fn test_pam_user_from_passwd() {
        let user =
            PamUser::from_passwd("alice:x:1001:1001:Alice,,,:/home/alice:/bin/bash\n").unwrap();
        assert_eq!(
            user,
            PamUser {
                name: "alice".into(),
                uid: 1001,
                home: "/home/alice".into()
            }
        );
        assert!(PamUser::from_passwd("alice:x:notanumber:1001::/home/alice:/bin/bash").is_err());
        assert!(PamUser::from_passwd("garbage").is_err());
    }

    #[test]
    fn test_pam_user_config_file_path() {
        let user = PamUser {
            name: "alice".into(),
            uid: 1001,
            home: "/home/alice".into(),
        };
        // in tests, the configuration path does not depend on the home directory
        assert_eq!(user.config_file_path(), PathBuf::from(_CONFIG));
    }

    #[test]
    #[serial]
    fn test_configurations_load_for_pam_user() {
        use std::os::unix::fs::MetadataExt as _;
        clean_config_dir();
        let me = std::fs::metadata(".").unwrap().uid();
        let owner = PamUser {
            name: "owner".into(),
            uid: me,
            home: "/nonexistent".into(),
        };
        let other = PamUser {
            name: "other".into(),
            uid: me + 1,
            home: "/nonexistent".into(),
        };

        // no file: empty configuration
        assert!(
            Configurations::load_for(&owner)
                .unwrap()
                .devices()
                .is_empty()
        );

        let mut config = Configuration::new("/dev/video0");
        config.add_to_savelist(&make_xu(&mut make_rng()));
        config.save().unwrap();
        assert_eq!(Configurations::load_for(&owner).unwrap().devices().len(), 1);
        // the file is not owned by this user (nor root, unless the tests run as root)
        if me != 0 {
            assert!(Configurations::load_for(&other).is_err());
        }

        // a symlink is refused, even to a file owned by the user
        let config_file = config_file_path().unwrap();
        let target = config_file.with_extension("target");
        std::fs::rename(&config_file, &target).unwrap();
        std::os::unix::fs::symlink(std::fs::canonicalize(&target).unwrap(), &config_file).unwrap();
        assert!(Configurations::load_for(&owner).is_err());
    }

    #[test]
    #[serial]
    fn test_configuration_adopt_legacy() {
        clean_config_dir();
        let mut rng = make_rng();
        let (xu1, xu2, xu3) = (make_xu(&mut rng), make_xu(&mut rng), make_xu(&mut rng));

        // a legacy by-id configuration of the camera, and one of another camera
        let mut legacy = Configuration::new("/dev/v4l/by-id/usb-Camera-video-index0");
        legacy.add_to_savelist(&xu1);
        legacy.add_to_blacklist(&xu2);
        legacy.save().unwrap();
        let mut other = Configuration::new("/dev/v4l/by-id/usb-Other-video-index0");
        other.add_to_savelist(&xu3);
        other.save().unwrap();

        let configs = Configurations::load().unwrap();
        let id: DeviceId = "usb-04f2:b6d9-0001-if02-index0".parse().unwrap();
        let mut config = Configuration::empty(id.clone());
        config.add_to_savelist(&xu3); // already in the new configuration, kept
        config.adopt_legacy(&configs, |legacy| legacy.to_string().contains("usb-Camera"));

        assert_eq!(config.get_savelist().len(), 2);
        assert!(config.is_blacklisted(&xu2));
        config.save().unwrap();

        // the legacy configuration is replaced, the other one is kept
        let keys: Vec<String> = Configurations::load()
            .unwrap()
            .devices()
            .keys()
            .map(|k| k.to_string())
            .collect();
        assert_eq!(
            keys,
            [
                "usb-04f2:b6d9-0001-if02-index0",
                "/dev/v4l/by-id/usb-Other-video-index0"
            ]
        );
    }

    #[test]
    #[serial]
    fn test_configuration_device_not_found_is_identified_by_path() {
        clean_config_dir();
        let config = Configuration::new("/dev/video10");
        assert_eq!(
            config.device(),
            &DeviceId::Path(PathBuf::from("/dev/video10"))
        );
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
    fn test_configuration_add_to_savelist_replaces_value() {
        clean_config_dir();
        let mut xu = make_xu(&mut make_rng());
        let mut config = Configuration::new("/dev/video0");
        config.add_to_savelist(&xu);
        xu.cur_mut()[0] = xu.cur()[0].wrapping_add(1);
        config.add_to_savelist(&xu);
        let list = config.get_savelist();
        assert_eq!(list.len(), 1);
        assert_eq!(list[0].cur(), xu.cur());
    }

    #[test]
    #[serial]
    fn test_configuration_save_keeps_unreadable_file() {
        clean_config_dir();
        let config_file = config_file_path().unwrap();
        std::fs::create_dir_all(config_file.parent().unwrap()).unwrap();
        let garbage = "this is [ not toml";
        std::fs::write(&config_file, garbage).unwrap();

        assert!(Configurations::load().is_err());

        let mut config = Configuration::new("/dev/video0");
        config.add_to_savelist(&make_xu(&mut make_rng()));
        assert!(config.save().is_err());
        assert_eq!(std::fs::read_to_string(&config_file).unwrap(), garbage);
    }

    #[test]
    #[serial]
    fn test_configurations_load_missing_file_is_empty() {
        clean_config_dir();
        assert!(Configurations::load().unwrap().devices().is_empty());
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
        assert_eq!(loaded.device, config.device);
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
        assert_eq!(loaded.device, config.device);
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
        assert_eq!(loaded.device, config.device);
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
        assert_eq!(loaded.device, config.device);
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
        assert_eq!(loaded.device, config.device);
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
        assert_eq!(loaded.device, config.device);
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
