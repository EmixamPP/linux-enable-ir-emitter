//! Migration of the v6 configurations to the v7 format.
//!
//! In v6, each configured device has a YAML file in `/etc/linux-enable-ir-emitter/`,
//! named after its `/dev/v4l/by-path/` name. The file is a list of instructions:
//! ```yaml
//! - status: start
//!   unit: 14
//!   selector: 6
//!   current:
//!     - 1
//!     - 3
//!   initial:
//!     - 0
//!     - 0
//!   maximum:
//!     - 3
//!     - 3
//! ```
//! The files ending with `.ini` contain the initial values of the controls, and are ignored.
//!
//! Before v6.1, the `status` field did not exist, and a `disable` boolean field was used instead.
//!
//! The files were always written by the same program, so instead of a full YAML parser,
//! a small strict parser of that format is used, which fails on anything else.

use crate::configuration::Configuration;
use crate::video::uvc::XuControl;

use std::collections::BTreeMap;
use std::path::{Path, PathBuf};

use anyhow::{Context, Result, anyhow, bail};

/// Default location of the v6 configurations.
pub const V6_CONFIG_DIR: &str = "/etc/linux-enable-ir-emitter";
/// Directory of the device paths used to name the v6 configurations.
const V6_DEVICE_DIR: &str = "/dev/v4l/by-path";

/// What to do with a v6 instruction in the v7 configuration.
#[derive(Debug, PartialEq, Eq)]
enum Migration {
    /// Saved, so applied by the `run` command.
    Save,
    /// Blacklisted, so never modified by the IR Enabler.
    Blacklist,
    /// Not migrated, because v6 did not apply it.
    Ignore,
}

/// A v6 instruction, i.e. a control of the device with its status.
#[derive(Debug, Default, PartialEq, Eq)]
struct Instruction {
    /// Since v6.1: `start`, `idle` or `disable`.
    status: Option<String>,
    /// Before v6.1: `true` if the instruction is disabled.
    disable: Option<bool>,
    unit: u8,
    selector: u8,
    current: Vec<u8>,
    /// Only present if different from `current`.
    initial: Option<Vec<u8>>,
}

impl Instruction {
    /// Returns what to do with the instruction, following how v6 applied it.
    fn migration(&self) -> Result<Migration> {
        if let Some(status) = &self.status {
            // v6.1: only the `start` instructions are applied
            match status.as_str() {
                "start" => Ok(Migration::Save),
                "idle" => Ok(Migration::Ignore),
                "disable" => Ok(Migration::Blacklist),
                other => bail!("unknown status {other:?}"),
            }
        } else if let Some(disable) = self.disable {
            // before v6.1: all the not disabled instructions are applied,
            // but only the modified ones (with an `initial` value) change something
            Ok(match (disable, &self.initial) {
                (true, _) => Migration::Blacklist,
                (false, Some(_)) => Migration::Save,
                (false, None) => Migration::Ignore,
            })
        } else {
            // v6.1 considers an instruction without status as `start`
            Ok(Migration::Save)
        }
    }

    fn to_control(&self) -> Result<XuControl> {
        Ok(XuControl::new(
            self.unit,
            self.selector,
            self.current.clone(),
            None,
            None,
            None,
            None,
            true,
        )?)
    }
}

/// A value of a field of a v6 instruction.
#[derive(Debug)]
enum Value {
    Scalar(String),
    List(Vec<String>),
}

/// Parses the content of a v6 configuration file.
fn parse(content: &str) -> Result<Vec<Instruction>> {
    let mut items: Vec<BTreeMap<String, Value>> = Vec::new();
    // key of the current item whose block list is being read, with its indentation
    let mut list_key: Option<(String, usize)> = None;
    let mut empty_document = false;

    for (number, raw_line) in content.lines().enumerate() {
        let error = |msg: &str| anyhow!("line {}: {msg}: {raw_line:?}", number + 1);

        let line = strip_comment(raw_line).trim_end();
        let content = line.trim_start();
        if content.is_empty() || content == "---" || content == "..." {
            continue;
        }
        if empty_document {
            return Err(error("unexpected content after an empty list"));
        }
        let indent = line.len() - content.len();

        if indent == 0 {
            if content == "[]" && items.is_empty() {
                empty_document = true;
                continue;
            }
            // a new instruction, its first field can be on the same line
            let rest = content
                .strip_prefix('-')
                .ok_or_else(|| error("expected an instruction starting with '-'"))?;
            items.push(BTreeMap::new());
            list_key = None;
            let rest = rest.trim();
            if !rest.is_empty() {
                // the field starts after "- "
                let field_indent = line.len() - rest.len();
                list_key = parse_field(rest, field_indent, items.last_mut().unwrap())
                    .map_err(|e| error(&e.to_string()))?;
            }
            continue;
        }

        let item = items
            .last_mut()
            .ok_or_else(|| error("expected an instruction starting with '-'"))?;

        if let Some(element) = content.strip_prefix('-') {
            // an element of a block list
            let key = match &list_key {
                Some((key, key_indent)) if indent >= *key_indent => key.clone(),
                _ => return Err(error("list element without a list field")),
            };
            if let Some(Value::List(list)) = item.get_mut(&key) {
                list.push(element.trim().to_string());
            }
            continue;
        }

        list_key = parse_field(content, indent, item).map_err(|e| error(&e.to_string()))?;
    }

    items
        .into_iter()
        .enumerate()
        .map(|(i, fields)| {
            to_instruction(fields).with_context(|| format!("invalid instruction {}", i + 1))
        })
        .collect()
}

/// Parses a `key: value` field into `item`.
///
/// Returns the key if the value is a block list, whose elements are on the following lines.
fn parse_field(
    field: &str,
    indent: usize,
    item: &mut BTreeMap<String, Value>,
) -> Result<Option<(String, usize)>> {
    let (key, value) = field
        .split_once(':')
        .context("expected a 'key: value' field")?;
    let key = key.trim().to_string();
    let value = value.trim();

    if item.contains_key(&key) {
        bail!("duplicated field {key:?}");
    }

    if value.is_empty() {
        item.insert(key.clone(), Value::List(Vec::new()));
        Ok(Some((key, indent)))
    } else if let Some(list) = value.strip_prefix('[') {
        let list = list
            .strip_suffix(']')
            .context("unterminated list, expected ']'")?;
        let elements = list
            .split(',')
            .map(|e| e.trim().to_string())
            .filter(|e| !e.is_empty())
            .collect();
        item.insert(key, Value::List(elements));
        Ok(None)
    } else {
        item.insert(key, Value::Scalar(unquote(value).to_string()));
        Ok(None)
    }
}

/// Converts the fields of an instruction.
fn to_instruction(mut fields: BTreeMap<String, Value>) -> Result<Instruction> {
    let mut scalar = |key: &str| -> Result<Option<String>> {
        match fields.remove(key) {
            None => Ok(None),
            Some(Value::Scalar(s)) => Ok(Some(s)),
            Some(Value::List(_)) => bail!("field {key:?} must not be a list"),
        }
    };
    let status = scalar("status")?;
    let disable = scalar("disable")?
        .map(|d| match d.as_str() {
            "true" => Ok(true),
            "false" => Ok(false),
            other => bail!("field \"disable\" must be true or false, not {other:?}"),
        })
        .transpose()?;
    let unit = scalar("unit")?.context("missing field \"unit\"")?;
    let selector = scalar("selector")?.context("missing field \"selector\"")?;

    let mut bytes = |key: &str| -> Result<Option<Vec<u8>>> {
        match fields.remove(key) {
            None => Ok(None),
            Some(Value::Scalar(_)) => bail!("field {key:?} must be a list"),
            Some(Value::List(list)) => list
                .iter()
                .map(|b| parse_byte(b).with_context(|| format!("invalid value in {key:?}")))
                .collect::<Result<Vec<u8>>>()
                .map(Some),
        }
    };
    let current = bytes("current")?.context("missing field \"current\"")?;
    if current.is_empty() {
        bail!("field \"current\" must not be empty");
    }
    let initial = bytes("initial")?;
    // maximum, minimum and the unknown fields are not needed in v7

    Ok(Instruction {
        status,
        disable,
        unit: parse_byte(&unit).context("invalid \"unit\"")?,
        selector: parse_byte(&selector).context("invalid \"selector\"")?,
        current,
        initial,
    })
}

fn parse_byte(value: &str) -> Result<u8> {
    unquote(value)
        .parse()
        .with_context(|| format!("{value:?} is not a value between 0 and 255"))
}

fn unquote(value: &str) -> &str {
    value
        .strip_prefix('"')
        .and_then(|v| v.strip_suffix('"'))
        .or_else(|| value.strip_prefix('\'').and_then(|v| v.strip_suffix('\'')))
        .unwrap_or(value)
}

/// Removes a `#` comment. The v6 values never contain a `#`.
fn strip_comment(line: &str) -> &str {
    line.split_once('#').map_or(line, |(before, _)| before)
}

/// Result of the migration of one v6 configuration file.
#[derive(Debug, PartialEq, Eq)]
enum Outcome {
    /// Migrated, with the number of saved and blacklisted controls.
    Migrated {
        device: String,
        saved: usize,
        blacklisted: usize,
    },
    /// The device already has a v7 configuration, which is kept.
    AlreadyConfigured(String),
    /// No instruction was applied by v6.
    NothingToMigrate,
}

/// Migrates all the v6 configurations found in `from` to the v7 configuration.
///
/// The devices must be connected, to find their persistent v7 path.
/// A device that already has a v7 configuration is skipped, so the migration can be run again safely.
pub fn migrate(from: &Path) -> Result<()> {
    migrate_from(from, Path::new(V6_DEVICE_DIR))
}

fn migrate_from(from: &Path, device_dir: &Path) -> Result<()> {
    let mut files: Vec<PathBuf> = std::fs::read_dir(from)
        .with_context(|| format!("failed to read the v6 configurations in {}", from.display()))?
        .filter_map(|entry| entry.ok().map(|e| e.path()))
        .filter(|path| path.is_file() && path.extension().is_none_or(|ext| ext != "ini"))
        .collect();
    files.sort();

    if files.is_empty() {
        println!("No v6 configuration found in {}.", from.display());
        return Ok(());
    }

    let mut failures = 0;
    for file in files {
        match migrate_file(&file, device_dir) {
            Ok(Outcome::Migrated {
                device,
                saved,
                blacklisted,
            }) => println!(
                "Migrated {}: {saved} saved and {blacklisted} blacklisted control(s) for {}.",
                file.display(),
                device
            ),
            Ok(Outcome::AlreadyConfigured(device)) => println!(
                "Skipped {}: {} already has a v7 configuration.",
                file.display(),
                device
            ),
            Ok(Outcome::NothingToMigrate) => {
                println!("Skipped {}: nothing to migrate.", file.display())
            }
            Err(err) => {
                eprintln!("Failed to migrate {}: {err:#}.", file.display());
                failures += 1;
            }
        }
    }

    if failures > 0 {
        bail!("failed to migrate {failures} configuration(s)");
    }
    Ok(())
}

fn migrate_file(file: &Path, device_dir: &Path) -> Result<Outcome> {
    let name = file.file_name().context("invalid file name")?;
    let device = device_dir.join(name);
    if !device.exists() {
        bail!(
            "the device {} is not connected, connect it and run the migration again",
            device.display()
        );
    }

    let content = std::fs::read_to_string(file).context("failed to read the file")?;
    let instructions = parse(&content)?;

    let mut configuration = Configuration::new(&device);
    if !configuration.is_empty() {
        return Ok(Outcome::AlreadyConfigured(
            configuration.device().to_string(),
        ));
    }

    let (mut saved, mut blacklisted) = (0, 0);
    for instruction in &instructions {
        let migration = instruction.migration().with_context(|| {
            format!(
                "invalid instruction unit {} selector {}",
                instruction.unit, instruction.selector
            )
        })?;
        match migration {
            Migration::Save => {
                configuration.add_to_savelist(&instruction.to_control()?);
                saved += 1;
            }
            Migration::Blacklist => {
                configuration.add_to_blacklist(&instruction.to_control()?);
                blacklisted += 1;
            }
            Migration::Ignore => {}
        }
    }

    if configuration.is_empty() {
        return Ok(Outcome::NothingToMigrate);
    }
    configuration.save()?;
    Ok(Outcome::Migrated {
        device: configuration.device().to_string(),
        saved,
        blacklisted,
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::configuration::_CONFIG;
    use serial_test::serial;

    /// As written by v6.1 (yaml-cpp block style).
    const V6_1: &str = "\
- status: start
  unit: 14
  selector: 6
  current:
    - 1
    - 3
  initial:
    - 0
    - 0
  maximum:
    - 3
    - 3
- status: idle
  unit: 4
  selector: 2
  current:
    - 7
- status: disable
  unit: 5
  selector: 1
  current:
    - 0
  minimum:
    - 0
";

    /// As written before v6.1, with the `disable` field.
    const V6_0: &str = "\
- disable: false
  unit: 14
  selector: 6
  current:
    - 1
  initial:
    - 0
- disable: false
  unit: 4
  selector: 2
  current:
    - 7
- disable: true
  unit: 5
  selector: 1
  current:
    - 0
";

    #[test]
    fn test_parse_v6_1() {
        let instructions = parse(V6_1).unwrap();
        assert_eq!(instructions.len(), 3);
        assert_eq!(
            instructions[0],
            Instruction {
                status: Some("start".into()),
                disable: None,
                unit: 14,
                selector: 6,
                current: vec![1, 3],
                initial: Some(vec![0, 0]),
            }
        );
        let migrations: Vec<_> = instructions
            .iter()
            .map(|i| i.migration().unwrap())
            .collect();
        assert_eq!(
            migrations,
            [Migration::Save, Migration::Ignore, Migration::Blacklist]
        );
    }

    #[test]
    fn test_parse_v6_0() {
        let migrations: Vec<_> = parse(V6_0)
            .unwrap()
            .iter()
            .map(|i| i.migration().unwrap())
            .collect();
        assert_eq!(
            migrations,
            [Migration::Save, Migration::Ignore, Migration::Blacklist]
        );
    }

    #[test]
    fn test_parse_flow_lists_quotes_and_comments() {
        let content = "---\n# hand edited\n- status: \"start\"  # applied\n  unit: '1'\n  selector: 2\n  current: [1, 2, 3]\n";
        let instructions = parse(content).unwrap();
        assert_eq!(instructions.len(), 1);
        assert_eq!(instructions[0].unit, 1);
        assert_eq!(instructions[0].current, vec![1, 2, 3]);
        assert_eq!(instructions[0].migration().unwrap(), Migration::Save);
    }

    #[test]
    fn test_parse_compact_block_list() {
        // a block list can be at the same indentation as its key
        let content = "- unit: 1\n  selector: 2\n  current:\n  - 4\n  - 5\n";
        let instructions = parse(content).unwrap();
        assert_eq!(instructions[0].current, vec![4, 5]);
        // no status: considered as start by v6.1
        assert_eq!(instructions[0].migration().unwrap(), Migration::Save);
    }

    #[test]
    fn test_parse_empty() {
        assert!(parse("[]\n").unwrap().is_empty());
        assert!(parse("").unwrap().is_empty());
    }

    #[test]
    fn test_parse_errors() {
        for content in [
            "unit: 1",                                               // not a list
            "- unit: 1\n  selector: 2\n",                            // missing current
            "- unit: 1\n  selector: 2\n  current: []\n",             // empty current
            "- unit: 256\n  selector: 2\n  current: [1]\n",          // not a byte
            "- unit: 1\n  selector: 2\n  current: [1, -1]\n",        // not a byte
            "- unit: 1\n  unit: 2\n  selector: 2\n  current: [1]\n", // duplicated field
            "- unit: 1\n  selector: 2\n  current: [1\n",             // unterminated list
            "- unit: 1\n  selector: 2\n  current: 1\n",              // not a list
            "- unit: [1]\n  selector: 2\n  current: [1]\n",          // not a scalar
            "- disable: maybe\n  unit: 1\n  selector: 2\n  current: [1]\n",
            "[]\n- unit: 1\n",
        ] {
            assert!(parse(content).is_err(), "should fail: {content:?}");
        }
    }

    #[test]
    fn test_unknown_status() {
        let instructions =
            parse("- status: on\n  unit: 1\n  selector: 2\n  current: [1]\n").unwrap();
        assert!(instructions[0].migration().is_err());
    }

    /// Creates a v6 configuration directory and a fake device directory.
    fn make_dirs(name: &str) -> (PathBuf, PathBuf) {
        let root = Path::new("target/tmp/migrate").join(name);
        let _ = std::fs::remove_dir_all(&root);
        let from = root.join("etc");
        let devices = root.join("by-path");
        std::fs::create_dir_all(&from).unwrap();
        std::fs::create_dir_all(&devices).unwrap();
        (from, devices)
    }

    #[test]
    #[serial]
    fn test_migrate() {
        let _ = std::fs::remove_file(_CONFIG);
        let (from, devices) = make_dirs("migrate");
        std::fs::write(from.join("pci-0000:00:14.0-usb-0:5:1.2-video-index0"), V6_1).unwrap();
        std::fs::write(
            from.join("pci-0000:00:14.0-usb-0:5:1.2-video-index0.ini"),
            "garbage",
        )
        .unwrap();
        let device = devices.join("pci-0000:00:14.0-usb-0:5:1.2-video-index0");
        std::fs::write(&device, "").unwrap();

        migrate_from(&from, &devices).unwrap();

        let configuration = Configuration::new(&device);
        let saved = configuration.get_savelist();
        assert_eq!(saved.len(), 1);
        assert_eq!((saved[0].unit(), saved[0].selector()), (14, 6));
        assert_eq!(saved[0].cur(), &[1, 3]);
        let blacklisted = XuControl::new(5, 1, vec![0], None, None, None, None, true).unwrap();
        assert!(configuration.is_blacklisted(&blacklisted));

        // running it again keeps the v7 configuration
        assert_eq!(
            migrate_file(
                &from.join("pci-0000:00:14.0-usb-0:5:1.2-video-index0"),
                &devices
            )
            .unwrap(),
            Outcome::AlreadyConfigured(device.display().to_string())
        );
    }

    #[test]
    #[serial]
    fn test_migrate_disconnected_device() {
        let _ = std::fs::remove_file(_CONFIG);
        let (from, devices) = make_dirs("disconnected");
        std::fs::write(from.join("connected"), V6_1).unwrap();
        std::fs::write(devices.join("connected"), "").unwrap();
        std::fs::write(from.join("disconnected"), V6_1).unwrap();

        let err = migrate_from(&from, &devices).unwrap_err();
        assert_eq!(err.to_string(), "failed to migrate 1 configuration(s)");
        // the connected device is still migrated
        assert_eq!(
            Configuration::new(devices.join("connected"))
                .get_savelist()
                .len(),
            1
        );
    }

    #[test]
    #[serial]
    fn test_migrate_nothing_to_migrate() {
        let _ = std::fs::remove_file(_CONFIG);
        let (from, devices) = make_dirs("nothing");
        let idle_only = "- status: idle\n  unit: 1\n  selector: 2\n  current: [1]\n";
        std::fs::write(from.join("cam"), idle_only).unwrap();
        std::fs::write(devices.join("cam"), "").unwrap();
        assert_eq!(
            migrate_file(&from.join("cam"), &devices).unwrap(),
            Outcome::NothingToMigrate
        );
    }

    #[test]
    fn test_migrate_missing_directory() {
        assert!(migrate(Path::new("target/tmp/migrate/does-not-exist")).is_err());
    }
}
