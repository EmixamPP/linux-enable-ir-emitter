//! Persistent identification of a V4L video device.
//!
//! The udev symlinks are not reliable to identify an infrared camera:
//! - `/dev/v4l/by-id/` does not contain the USB interface, so when a camera exposes its RGB and IR
//!   sensors on two interfaces of the same USB device, both get the same name, and it can point to either one.
//! - `/dev/v4l/by-path/` depends on where the device is plugged and on the udev naming format, which changed over time.
//!
//! Instead, a USB video device is identified by its vendor and product IDs, its serial number (if any),
//! its interface number and its V4L index, read from sysfs. E.g. `usb-0bda:5556-200901010001-if02-index0`.
//! Any other device is identified by its path.

use std::fmt;
use std::path::{Path, PathBuf};

use anyhow::{Context, Result, bail};
use serde::{Deserialize, Serialize};

const SYSFS_V4L_DIR: &str = "/sys/class/video4linux";
const DEV_DIR: &str = "/dev";

/// Identifier of a video device.
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub enum DeviceId {
    /// A USB video device, identified independently of its device node.
    Usb(UsbId),
    /// Any other device, or a configuration saved before the USB identifiers, identified by its path.
    Path(PathBuf),
}

/// Identifier of a USB video device.
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct UsbId {
    vendor: String,
    product: String,
    serial: Option<String>,
    interface: String,
    index: String,
}

impl Default for DeviceId {
    fn default() -> Self {
        Self::Path(PathBuf::new())
    }
}

impl DeviceId {
    /// Identifies the device at `device`, which can be any path to the device node (e.g. a symlink).
    ///
    /// If the device is not a USB video device, or does not exist, it is identified by the given path.
    pub fn new(device: &Path) -> Self {
        Self::new_from(Path::new(SYSFS_V4L_DIR), device)
    }

    fn new_from(sysfs_dir: &Path, device: &Path) -> Self {
        device
            .canonicalize()
            .ok()
            .and_then(|node| UsbId::from_sysfs(sysfs_dir, node.file_name()?.to_str()?))
            .map_or_else(|| Self::Path(device.to_path_buf()), Self::Usb)
    }

    /// Returns the current path of the device node, or `None` if the device is not connected.
    pub fn resolve(&self) -> Option<PathBuf> {
        self.resolve_from(Path::new(SYSFS_V4L_DIR), Path::new(DEV_DIR))
    }

    fn resolve_from(&self, sysfs_dir: &Path, dev_dir: &Path) -> Option<PathBuf> {
        let usb_id = match self {
            Self::Path(path) => return path.exists().then(|| path.clone()),
            Self::Usb(usb_id) => usb_id,
        };

        let mut nodes: Vec<String> = std::fs::read_dir(sysfs_dir)
            .ok()?
            .filter_map(|entry| entry.ok()?.file_name().into_string().ok())
            .filter(|node| UsbId::from_sysfs(sysfs_dir, node).as_ref() == Some(usb_id))
            .collect();
        nodes.sort();
        if nodes.len() > 1 {
            log::warn!(
                "Several devices match {self}: {nodes:?}, using the first one. They are probably identical cameras without serial number."
            );
        }
        nodes.first().map(|node| dev_dir.join(node))
    }

    /// Returns true if this identifies the device at `device`.
    ///
    /// For a device identified by its path, the paths are compared after resolving the symlinks,
    /// so `/dev/videoX` matches a configuration saved under a `/dev/v4l/...` symlink to it.
    pub fn matches(&self, device: &Path) -> bool {
        self.matches_from(Path::new(SYSFS_V4L_DIR), device)
    }

    fn matches_from(&self, sysfs_dir: &Path, device: &Path) -> bool {
        if *self == Self::new_from(sysfs_dir, device) {
            return true;
        }
        match self {
            Self::Path(path) => match (path.canonicalize(), device.canonicalize()) {
                (Ok(a), Ok(b)) => a == b,
                _ => false,
            },
            Self::Usb(_) => false,
        }
    }

    /// Returns true if both identify a node of the same USB device, whatever its interface or index.
    ///
    /// A device identified by its path is resolved to its current device node first.
    pub fn same_usb_device(&self, other: &Self) -> bool {
        self.same_usb_device_from(Path::new(SYSFS_V4L_DIR), other)
    }

    fn same_usb_device_from(&self, sysfs_dir: &Path, other: &Self) -> bool {
        let to_usb = |id: &Self| match id {
            Self::Usb(usb_id) => Some(usb_id.clone()),
            Self::Path(path) => match Self::new_from(sysfs_dir, path) {
                Self::Usb(usb_id) => Some(usb_id),
                Self::Path(_) => None,
            },
        };
        match (to_usb(self), to_usb(other)) {
            (Some(a), Some(b)) => {
                a.vendor == b.vendor && a.product == b.product && a.serial == b.serial
            }
            _ => false,
        }
    }
}

impl UsbId {
    /// Reads the identifier of the video node `node` (e.g. `video2`) from sysfs.
    ///
    /// Returns `None` if the node is not a USB video device.
    fn from_sysfs(sysfs_dir: &Path, node: &str) -> Option<Self> {
        let read = |path: PathBuf| -> Option<String> {
            let value = std::fs::read_to_string(path).ok()?;
            let value = value.trim();
            (!value.is_empty()).then(|| value.to_string())
        };

        let node_dir = sysfs_dir.join(node);
        let index = read(node_dir.join("index"))?;
        // the device of a USB video node is its USB interface, whose parent is the USB device
        let interface_dir = node_dir.join("device").canonicalize().ok()?;
        let interface = read(interface_dir.join("bInterfaceNumber"))?;
        let usb_dir = interface_dir.parent()?;
        Some(Self {
            vendor: read(usb_dir.join("idVendor"))?,
            product: read(usb_dir.join("idProduct"))?,
            serial: read(usb_dir.join("serial")),
            interface,
            index,
        })
    }

    fn parse(id: &str) -> Result<Self> {
        let invalid = || format!("invalid USB device identifier {id:?}");
        let rest = id.strip_prefix("usb-").with_context(invalid)?;
        // the serial can contain '-', so split the interface and index from the end
        let (rest, index) = rest.rsplit_once("-index").with_context(invalid)?;
        let (rest, interface) = rest.rsplit_once("-if").with_context(invalid)?;
        let (ids, serial) = match rest.split_once('-') {
            Some((ids, serial)) => (ids, Some(serial.to_string())),
            None => (rest, None),
        };
        let (vendor, product) = ids.split_once(':').with_context(invalid)?;
        if [vendor, product, interface, index]
            .iter()
            .any(|v| v.is_empty())
            || serial.as_ref().is_some_and(|s| s.is_empty())
        {
            bail!(invalid());
        }
        Ok(Self {
            vendor: vendor.to_string(),
            product: product.to_string(),
            serial,
            interface: interface.to_string(),
            index: index.to_string(),
        })
    }
}

impl fmt::Display for DeviceId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Usb(id) => {
                write!(f, "usb-{}:{}", id.vendor, id.product)?;
                if let Some(serial) = &id.serial {
                    write!(f, "-{serial}")?;
                }
                write!(f, "-if{}-index{}", id.interface, id.index)
            }
            Self::Path(path) => write!(f, "{}", path.display()),
        }
    }
}

impl std::str::FromStr for DeviceId {
    type Err = anyhow::Error;

    fn from_str(id: &str) -> Result<Self> {
        if id.starts_with("usb-") {
            Ok(Self::Usb(UsbId::parse(id)?))
        } else {
            Ok(Self::Path(PathBuf::from(id)))
        }
    }
}

impl Serialize for DeviceId {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        serializer.serialize_str(&self.to_string())
    }
}

impl<'de> Deserialize<'de> for DeviceId {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let id = String::deserialize(deserializer)?;
        id.parse().map_err(serde::de::Error::custom)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::os::unix::fs::symlink;

    /// A fake sysfs and /dev, with the RGB and IR interfaces of one camera, like the Chicony cameras:
    /// - video0 and video1: interface 00 (RGB)
    /// - video2 and video3: interface 02 (IR)
    /// - video4: a non USB device
    struct FakeSystem {
        sysfs: PathBuf,
        dev: PathBuf,
    }

    impl FakeSystem {
        fn new(name: &str, serial: Option<&str>) -> Self {
            let root = Path::new("target/tmp/device_id").join(name);
            let _ = std::fs::remove_dir_all(&root);
            let sysfs = root.join("sys/class/video4linux");
            let dev = root.join("dev");
            let usb = root.join("sys/devices/usb1/1-3");
            std::fs::create_dir_all(&sysfs).unwrap();
            std::fs::create_dir_all(&dev).unwrap();
            std::fs::create_dir_all(&usb).unwrap();
            std::fs::write(usb.join("idVendor"), "04f2\n").unwrap();
            std::fs::write(usb.join("idProduct"), "b6d9\n").unwrap();
            if let Some(serial) = serial {
                std::fs::write(usb.join("serial"), format!("{serial}\n")).unwrap();
            }

            for (node, interface, index) in [
                ("video0", "00", "0"),
                ("video1", "00", "1"),
                ("video2", "02", "0"),
                ("video3", "02", "1"),
            ] {
                let interface_dir = usb.join(format!(
                    "1-3:1.{}",
                    interface.trim_start_matches('0').parse::<u8>().unwrap_or(0)
                ));
                std::fs::create_dir_all(&interface_dir).unwrap();
                std::fs::write(
                    interface_dir.join("bInterfaceNumber"),
                    format!("{interface}\n"),
                )
                .unwrap();
                let node_dir = sysfs.join(node);
                std::fs::create_dir_all(&node_dir).unwrap();
                std::fs::write(node_dir.join("index"), format!("{index}\n")).unwrap();
                symlink(
                    std::fs::canonicalize(&interface_dir).unwrap(),
                    node_dir.join("device"),
                )
                .unwrap();
                std::fs::write(dev.join(node), "").unwrap();
            }

            // a non USB device, e.g. a platform camera
            let platform = root.join("sys/devices/platform/cam");
            std::fs::create_dir_all(&platform).unwrap();
            let node_dir = sysfs.join("video4");
            std::fs::create_dir_all(&node_dir).unwrap();
            std::fs::write(node_dir.join("index"), "0\n").unwrap();
            symlink(
                std::fs::canonicalize(&platform).unwrap(),
                node_dir.join("device"),
            )
            .unwrap();
            std::fs::write(dev.join("video4"), "").unwrap();

            Self { sysfs, dev }
        }

        fn id(&self, node: &str) -> DeviceId {
            DeviceId::new_from(&self.sysfs, &self.dev.join(node))
        }
    }

    #[test]
    fn test_usb_id_contains_interface_and_index() {
        let system = FakeSystem::new("ids", Some("0001"));
        assert_eq!(
            system.id("video0").to_string(),
            "usb-04f2:b6d9-0001-if00-index0"
        );
        assert_eq!(
            system.id("video1").to_string(),
            "usb-04f2:b6d9-0001-if00-index1"
        );
        // the IR interface is distinguished from the RGB one, unlike /dev/v4l/by-id
        assert_eq!(
            system.id("video2").to_string(),
            "usb-04f2:b6d9-0001-if02-index0"
        );
    }

    #[test]
    fn test_usb_id_without_serial() {
        let system = FakeSystem::new("no_serial", None);
        assert_eq!(system.id("video2").to_string(), "usb-04f2:b6d9-if02-index0");
    }

    #[test]
    fn test_symlink_is_identified_as_its_target() {
        let system = FakeSystem::new("symlink", Some("0001"));
        let link = system.dev.join("by-id-link");
        symlink(
            std::fs::canonicalize(system.dev.join("video2")).unwrap(),
            &link,
        )
        .unwrap();
        assert_eq!(
            DeviceId::new_from(&system.sysfs, &link),
            system.id("video2")
        );
    }

    #[test]
    fn test_non_usb_and_missing_devices_are_identified_by_path() {
        let system = FakeSystem::new("non_usb", Some("0001"));
        assert_eq!(
            system.id("video4"),
            DeviceId::Path(system.dev.join("video4"))
        );
        assert_eq!(
            system.id("video9"),
            DeviceId::Path(system.dev.join("video9"))
        );
    }

    #[test]
    fn test_resolve() {
        let system = FakeSystem::new("resolve", Some("0001"));
        let ir = system.id("video2");
        assert_eq!(
            ir.resolve_from(&system.sysfs, &system.dev),
            Some(system.dev.join("video2"))
        );
        let unknown: DeviceId = "usb-dead:beef-if00-index0".parse().unwrap();
        assert_eq!(unknown.resolve_from(&system.sysfs, &system.dev), None);
        let path = DeviceId::Path(system.dev.join("video4"));
        assert_eq!(
            path.resolve_from(&system.sysfs, &system.dev),
            Some(system.dev.join("video4"))
        );
        let missing = DeviceId::Path(system.dev.join("video9"));
        assert_eq!(missing.resolve_from(&system.sysfs, &system.dev), None);
    }

    #[test]
    fn test_matches() {
        let system = FakeSystem::new("matches", Some("0001"));
        let ir = system.id("video2");
        assert!(ir.matches_from(&system.sysfs, &system.dev.join("video2")));
        assert!(!ir.matches_from(&system.sysfs, &system.dev.join("video0")));

        // a legacy configuration saved under a symlink matches its current target
        let link = system.dev.join("legacy-link");
        symlink(
            std::fs::canonicalize(system.dev.join("video2")).unwrap(),
            &link,
        )
        .unwrap();
        let legacy = DeviceId::Path(link);
        assert!(legacy.matches_from(&system.sysfs, &system.dev.join("video2")));
    }

    #[test]
    fn test_same_usb_device() {
        let system = FakeSystem::new("same", Some("0001"));
        let rgb = system.id("video0");
        let ir = system.id("video2");
        assert!(ir.same_usb_device_from(&system.sysfs, &rgb));
        // a legacy path pointing to the RGB node, as with the /dev/v4l/by-id collision
        let legacy = DeviceId::Path(system.dev.join("video0"));
        assert!(ir.same_usb_device_from(&system.sysfs, &legacy));
        let platform = system.id("video4");
        assert!(!ir.same_usb_device_from(&system.sysfs, &platform));
    }

    #[test]
    fn test_parse_and_display() {
        for id in [
            "usb-04f2:b6d9-0001-if02-index0",
            "usb-04f2:b6d9-if02-index0",
            "usb-04f2:b6d9-SN-with-if-dashes-index-if02-index1",
            "/dev/v4l/by-id/usb-Chicony_Integrated_Camera-video-index0",
            "/dev/video2",
            "relative/path/video2",
        ] {
            assert_eq!(id.parse::<DeviceId>().unwrap().to_string(), id);
        }
        let with_dashes: DeviceId = "usb-04f2:b6d9-SN-with-if-dashes-index-if02-index1"
            .parse()
            .unwrap();
        match with_dashes {
            DeviceId::Usb(id) => {
                assert_eq!(id.serial.as_deref(), Some("SN-with-if-dashes-index"));
                assert_eq!((id.interface.as_str(), id.index.as_str()), ("02", "1"));
            }
            DeviceId::Path(_) => panic!("expected a USB identifier"),
        }
    }

    #[test]
    fn test_parse_invalid() {
        for id in [
            "usb-04f2b6d9-if02-index0",
            "usb-04f2:b6d9-if02",
            "usb-04f2:b6d9-index0",
            "usb-:b6d9-if02-index0",
            "usb-04f2:b6d9--if02-index0",
        ] {
            assert!(id.parse::<DeviceId>().is_err(), "should fail: {id}");
        }
    }

    #[test]
    fn test_serde() {
        let id: DeviceId = "usb-04f2:b6d9-0001-if02-index0".parse().unwrap();
        let toml = toml::to_string(&std::collections::BTreeMap::from([(id.clone(), 1)])).unwrap();
        assert_eq!(toml.trim(), "\"usb-04f2:b6d9-0001-if02-index0\" = 1");
        let back: std::collections::BTreeMap<DeviceId, i32> = toml::from_str(&toml).unwrap();
        assert_eq!(back.keys().next(), Some(&id));
    }
}
