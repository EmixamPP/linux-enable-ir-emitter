//! `setup-howdy` command: executes the tool before howdy in PAM, and allows it in the polkit password prompts.
//!
//! 1. In each file of `/etc/pam.d/` loading howdy, a `pam_exec` line executing `linux-enable-ir-emitter run`
//!    is added before the howdy line, preceded by a marker comment so it can be removed with `--undo`.
//! 2. Since polkit 126, the polkit password prompts are executed in a sandboxed systemd service without access to
//!    the video devices and the home directories. A drop-in allows them, which has no effect with an older polkit.

use std::path::{Path, PathBuf};

use anyhow::{Context, Result, bail};

/// Comment added before the PAM line, to identify it.
const MARKER: &str = "# Added by linux-enable-ir-emitter setup-howdy, remove with: linux-enable-ir-emitter setup-howdy --undo";
/// PAM module of howdy.
const HOWDY_MODULE: &str = "pam_howdy.so";
const POLKIT_UNIT: &str = "polkit-agent-helper@.service";
const POLKIT_DROPIN: &str = "linux-enable-ir-emitter.conf";
const POLKIT_DROPIN_CONTENT: &str = "\
# Added by linux-enable-ir-emitter setup-howdy, remove with: linux-enable-ir-emitter setup-howdy --undo
# Allows linux-enable-ir-emitter to enable the IR emitter, and howdy to use the camera, in the polkit password prompts.
[Service]
PrivateDevices=no
DeviceAllow=char-video4linux rw
ProtectHome=read-only
";

/// Locations modified by the command.
struct System {
    pam_dir: PathBuf,
    /// Drop-in directory of the polkit unit.
    dropin_dir: PathBuf,
    reload_systemd: bool,
}

impl System {
    fn real() -> Self {
        Self {
            pam_dir: PathBuf::from("/etc/pam.d"),
            dropin_dir: PathBuf::from(format!("/etc/systemd/system/{POLKIT_UNIT}.d")),
            reload_systemd: true,
        }
    }
}

/// Executes the `setup-howdy` command.
pub fn setup_howdy(undo: bool) -> Result<()> {
    if !rustix::process::geteuid().is_root() {
        bail!("this command must be executed as root, e.g. with sudo");
    }
    let system = System::real();
    if undo {
        undo_setup(&system)?;
    } else {
        let exe = std::env::current_exe().context("failed to find the path of the executable")?;
        setup(&system, &exe)?;
    }
    println!(
        "\nPlease keep this terminal open, and check in another terminal that you can still authenticate, e.g. with `sudo true`."
    );
    Ok(())
}

fn setup(system: &System, exe: &Path) -> Result<()> {
    let exe = exe
        .to_str()
        .context("the executable path is not valid UTF-8")?;
    if exe.contains(char::is_whitespace) {
        bail!("the executable path {exe:?} contains spaces, which is not supported by PAM");
    }
    let pam_line = format!("auth optional pam_exec.so quiet {exe} run");

    let mut howdy_found = false;
    for file in pam_files(&system.pam_dir)? {
        let content = std::fs::read_to_string(&file)
            .with_context(|| format!("failed to read {}", file.display()))?;
        match add_hook(&content, &pam_line).with_context(|| format!("in {}", file.display()))? {
            Edit::NoHowdy => {}
            Edit::AlreadyDone => {
                howdy_found = true;
                println!("{}: already set up.", file.display());
            }
            Edit::Changed {
                content: new,
                notes,
            } => {
                howdy_found = true;
                print_notes(&file, &notes);
                write_file(&file, &new)?;
            }
        }
    }
    if !howdy_found {
        bail!(
            "howdy is not used in any file of {}, please install and configure howdy first",
            system.pam_dir.display()
        );
    }

    let dropin = system.dropin_dir.join(POLKIT_DROPIN);
    if std::fs::read_to_string(&dropin).is_ok_and(|c| c == POLKIT_DROPIN_CONTENT) {
        println!("{}: already set up.", dropin.display());
        return Ok(());
    }
    std::fs::create_dir_all(&system.dropin_dir)
        .with_context(|| format!("failed to create {}", system.dropin_dir.display()))?;
    std::fs::write(&dropin, POLKIT_DROPIN_CONTENT)
        .with_context(|| format!("failed to write {}", dropin.display()))?;
    println!(
        "{}: written, to allow the polkit password prompts.",
        dropin.display()
    );
    reload_systemd(system);
    Ok(())
}

fn undo_setup(system: &System) -> Result<()> {
    for file in pam_files(&system.pam_dir)? {
        let content = std::fs::read_to_string(&file)
            .with_context(|| format!("failed to read {}", file.display()))?;
        if let Edit::Changed {
            content: new,
            notes,
        } = remove_hook(&content).with_context(|| format!("in {}", file.display()))?
        {
            print_notes(&file, &notes);
            write_file(&file, &new)?;
        }
    }

    let dropin = system.dropin_dir.join(POLKIT_DROPIN);
    if dropin.exists() {
        std::fs::remove_file(&dropin)
            .with_context(|| format!("failed to remove {}", dropin.display()))?;
        // only removed if empty
        let _ = std::fs::remove_dir(&system.dropin_dir);
        println!("{}: removed.", dropin.display());
        reload_systemd(system);
    }
    Ok(())
}

/// Returns the PAM configuration files.
fn pam_files(dir: &Path) -> Result<Vec<PathBuf>> {
    let mut files: Vec<PathBuf> = std::fs::read_dir(dir)
        .with_context(|| format!("failed to read {}", dir.display()))?
        .filter_map(|entry| entry.ok().map(|e| e.path()))
        .filter(|path| {
            let name = path.file_name().and_then(|n| n.to_str()).unwrap_or("");
            path.is_file() && !name.starts_with('.')
        })
        .collect();
    files.sort();
    Ok(files)
}

fn print_notes(file: &Path, notes: &[String]) {
    println!("{}: changed", file.display());
    for note in notes {
        println!("  {note}");
    }
}

/// Replaces the file content atomically, keeping its permissions.
fn write_file(file: &Path, content: &str) -> Result<()> {
    let name = file
        .file_name()
        .context("invalid file name")?
        .to_string_lossy();
    let tmp = file.with_file_name(format!(".{name}.linux-enable-ir-emitter.tmp"));
    let permissions = std::fs::metadata(file)
        .with_context(|| format!("failed to read {}", file.display()))?
        .permissions();
    std::fs::write(&tmp, content).with_context(|| format!("failed to write {}", tmp.display()))?;
    std::fs::set_permissions(&tmp, permissions)
        .with_context(|| format!("failed to set the permissions of {}", tmp.display()))?;
    std::fs::rename(&tmp, file).with_context(|| format!("failed to replace {}", file.display()))
}

fn reload_systemd(system: &System) {
    if !system.reload_systemd {
        return;
    }
    match std::process::Command::new("systemctl")
        .arg("daemon-reload")
        .status()
    {
        Ok(status) if status.success() => {}
        _ => eprintln!("Failed to reload systemd, please execute: systemctl daemon-reload"),
    }
}

/// Result of the edition of a PAM file.
#[derive(Debug, PartialEq, Eq)]
enum Edit {
    /// The file does not load howdy.
    NoHowdy,
    /// Nothing to change.
    AlreadyDone,
    /// The new content, with a description of the changes.
    Changed { content: String, notes: Vec<String> },
}

/// A rule of a PAM file, i.e. a line which is not empty nor a comment.
struct Rule<'a> {
    line: &'a str,
}

impl<'a> Rule<'a> {
    fn parse(line: &'a str) -> Option<Self> {
        let trimmed = line.trim();
        (!trimmed.is_empty() && !trimmed.starts_with('#')).then_some(Self { line })
    }

    /// Returns the type of the rule (`auth`, `account`, ...), without the `-` prefix.
    fn kind(&self) -> String {
        let first = self.line.split_whitespace().next().unwrap_or("");
        first.trim_start_matches('-').to_lowercase()
    }

    /// Returns true if the rule includes other rules, whose number is unknown.
    fn is_include(&self) -> bool {
        let mut tokens = self.line.split_whitespace();
        let first = tokens.next().unwrap_or("");
        first.starts_with("@include") || matches!(tokens.next(), Some("include" | "substack"))
    }

    fn is_howdy(&self) -> bool {
        self.line.contains(HOWDY_MODULE)
            || (self.line.contains("pam_python.so") && self.line.contains("howdy"))
    }

    /// Returns true if the rule executes this tool, added by this command or manually.
    fn is_ours(&self) -> bool {
        self.line.contains("pam_exec.so") && self.line.contains("linux-enable-ir-emitter")
    }

    /// Returns the byte range of the `[...]` control field, if any.
    fn control_range(&self) -> Option<(usize, usize)> {
        let start = self.line.find('[')?;
        let end = start + self.line[start..].find(']')?;
        Some((start, end + 1))
    }

    /// Returns the jumps `value=N` of the control field.
    fn jumps(&self) -> Vec<usize> {
        self.control_range()
            .map(|(start, end)| {
                self.line[start + 1..end - 1]
                    .split_whitespace()
                    .filter_map(|action| action.split_once('=')?.1.parse().ok())
                    .collect()
            })
            .unwrap_or_default()
    }
}

/// Adds the line of each jump `value=N` whose target is after the rule at `position` to `delta`.
///
/// `position` is the index in `lines` of the inserted rule (`delta` = 1), or of the removed rule (`delta` = -1).
/// The jumps are counted on the `auth` rules only, since each type has its own stack.
fn adjust_jumps(
    lines: &mut [String],
    position: usize,
    delta: isize,
    notes: &mut Vec<String>,
) -> Result<()> {
    // indices in `lines` of the auth rules before the position
    let auth_before: Vec<usize> = (0..position)
        .filter(|&i| Rule::parse(&lines[i]).is_some_and(|r| r.kind() == "auth"))
        .collect();
    // index of the inserted or removed rule among the auth rules
    let p = auth_before.len();

    for (k, &i) in auth_before.iter().enumerate() {
        let rule = Rule::parse(&lines[i]).unwrap();
        let Some((start, end)) = rule.control_range() else {
            continue;
        };
        if !rule.jumps().iter().any(|&n| k + n + 1 > p) {
            continue;
        }
        // the rule jumps over the position: the auth rules in between must be countable
        if (i + 1..position).any(|j| {
            Rule::parse(&lines[j]).is_some_and(|r| {
                r.is_include()
                    && (r.kind() == "auth" || r.line.trim_start().starts_with("@include"))
            })
        }) {
            bail!(
                "line {} jumps over an include, please add the line manually: {}",
                i + 1,
                lines[i].trim()
            );
        }

        let control: Vec<String> = lines[i][start + 1..end - 1]
            .split_whitespace()
            .map(|action| match action.split_once('=') {
                Some((value, n)) => match n.parse::<usize>() {
                    Ok(n) if k + n + 1 > p => format!("{value}={}", n.saturating_add_signed(delta)),
                    _ => action.to_string(),
                },
                None => action.to_string(),
            })
            .collect();
        let new = format!(
            "{}[{}]{}",
            &lines[i][..start],
            control.join(" "),
            &lines[i][end..]
        );
        notes.push(format!(
            "line {}: adjust the jump over the line: {}",
            i + 1,
            new.trim()
        ));
        lines[i] = new;
    }
    Ok(())
}

fn join(lines: &[String], original: &str) -> String {
    let mut content = lines.join("\n");
    if original.ends_with('\n') {
        content.push('\n');
    }
    content
}

/// Adds `pam_line` before the first howdy rule of a PAM file content.
fn add_hook(content: &str, pam_line: &str) -> Result<Edit> {
    let mut lines: Vec<String> = content.lines().map(String::from).collect();
    let rules: Vec<(usize, Rule)> = lines
        .iter()
        .enumerate()
        .filter_map(|(i, l)| Some((i, Rule::parse(l)?)))
        .collect();
    let Some(&(howdy, _)) = rules.iter().find(|(_, r)| r.is_howdy()) else {
        return Ok(Edit::NoHowdy);
    };
    if rules.iter().any(|(_, r)| r.is_ours()) {
        return Ok(Edit::AlreadyDone);
    }

    let mut notes = Vec::new();
    adjust_jumps(&mut lines, howdy, 1, &mut notes)?;
    lines.insert(howdy, pam_line.to_string());
    lines.insert(howdy, MARKER.to_string());
    notes.insert(0, format!("line {}: add {pam_line}", howdy + 1));
    Ok(Edit::Changed {
        content: join(&lines, content),
        notes,
    })
}

/// Removes the lines added by [`add_hook`] from a PAM file content.
fn remove_hook(content: &str) -> Result<Edit> {
    let mut lines: Vec<String> = content.lines().map(String::from).collect();
    let mut notes = Vec::new();
    while let Some(marker) = (0..lines.len()).find(|&i| {
        lines[i] == MARKER
            && lines
                .get(i + 1)
                .and_then(|l| Rule::parse(l))
                .is_some_and(|r| r.is_ours())
    }) {
        notes.push(format!(
            "line {}: remove {}",
            marker + 2,
            lines[marker + 1].trim()
        ));
        adjust_jumps(&mut lines, marker + 1, -1, &mut notes)?;
        lines.drain(marker..marker + 2);
    }
    if notes.is_empty() {
        return Ok(Edit::AlreadyDone);
    }
    Ok(Edit::Changed {
        content: join(&lines, content),
        notes,
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    const LINE: &str = "auth optional pam_exec.so quiet /usr/bin/linux-enable-ir-emitter run";

    fn changed(edit: Edit) -> String {
        match edit {
            Edit::Changed { content, .. } => content,
            other => panic!("expected a change, got {other:?}"),
        }
    }

    #[test]
    fn test_add_before_howdy() {
        let content = format!(
            "#%PAM-1.0\nauth sufficient {HOWDY_MODULE}\nauth include system-login\naccount include system-login\n"
        );
        let new = changed(add_hook(&content, LINE).unwrap());
        assert_eq!(
            new,
            format!(
                "#%PAM-1.0\n{MARKER}\n{LINE}\nauth sufficient {HOWDY_MODULE}\nauth include system-login\naccount include system-login\n"
            )
        );
    }

    #[test]
    fn test_add_before_old_howdy() {
        let content = "auth sufficient pam_python.so /lib/security/howdy/pam.py\n";
        let new = changed(add_hook(content, LINE).unwrap());
        assert!(new.starts_with(&format!("{MARKER}\n{LINE}\n")));
    }

    #[test]
    fn test_add_without_howdy_or_already_done() {
        assert_eq!(
            add_hook("auth required pam_unix.so\n", LINE).unwrap(),
            Edit::NoHowdy
        );
        // commented howdy line
        assert_eq!(
            add_hook(&format!("#auth sufficient {HOWDY_MODULE}\n"), LINE).unwrap(),
            Edit::NoHowdy
        );
        // already added, by this command or manually
        let manual = format!(
            "auth optional pam_exec.so /usr/local/bin/linux-enable-ir-emitter run --config /home/me/.config/linux-enable-ir-emitter.toml\nauth sufficient {HOWDY_MODULE}\n"
        );
        assert_eq!(add_hook(&manual, LINE).unwrap(), Edit::AlreadyDone);
        let done = changed(add_hook(&format!("auth sufficient {HOWDY_MODULE}\n"), LINE).unwrap());
        assert_eq!(add_hook(&done, LINE).unwrap(), Edit::AlreadyDone);
    }

    #[test]
    fn test_add_adjusts_jumps_over_the_line() {
        // e.g. Debian common-auth, with fprintd before howdy
        let content = format!(
            "\
auth\t[success=3 default=ignore]\tpam_fprintd.so max-tries=1
auth\t[success=2 default=ignore]\t{HOWDY_MODULE}
auth\t[success=1 default=ignore]\tpam_unix.so nullok
auth\trequisite\t\t\tpam_deny.so
auth\trequired\t\t\tpam_permit.so
"
        );
        let new = changed(add_hook(&content, LINE).unwrap());
        let lines: Vec<&str> = new.lines().collect();
        // fprintd still jumps to pam_permit
        assert_eq!(
            lines[0],
            "auth\t[success=4 default=ignore]\tpam_fprintd.so max-tries=1"
        );
        assert_eq!(lines[1], MARKER);
        assert_eq!(lines[2], LINE);
        // the lines after are not changed
        assert_eq!(
            lines[3],
            format!("auth\t[success=2 default=ignore]\t{HOWDY_MODULE}")
        );
        assert_eq!(
            lines[4],
            "auth\t[success=1 default=ignore]\tpam_unix.so nullok"
        );
    }

    #[test]
    fn test_add_keeps_jumps_not_over_the_line() {
        // a jump landing on howdy now lands on the added line, executed just before howdy
        let content = format!(
            "\
auth [success=1 default=ignore] pam_succeed_if.so user ingroup nohowdy
auth requisite pam_deny.so
auth sufficient {HOWDY_MODULE}
"
        );
        let new = changed(add_hook(&content, LINE).unwrap());
        assert!(new.starts_with("auth [success=1 default=ignore] pam_succeed_if.so"));
    }

    #[test]
    fn test_add_counts_only_auth_rules() {
        let content = format!(
            "\
auth [success=2 default=ignore] pam_fprintd.so
account required pam_unix.so
auth sufficient {HOWDY_MODULE}
auth required pam_unix.so
"
        );
        let new = changed(add_hook(&content, LINE).unwrap());
        // the jump goes over the added line, so it is incremented, and the account rule is not counted
        assert!(new.starts_with("auth [success=3 default=ignore] pam_fprintd.so"));
    }

    #[test]
    fn test_add_refuses_jump_over_include() {
        let content = format!(
            "\
auth [success=3 default=ignore] pam_fprintd.so
@include common-auth-extra
auth sufficient {HOWDY_MODULE}
auth required pam_unix.so
"
        );
        assert!(add_hook(&content, LINE).is_err());
    }

    #[test]
    fn test_remove_restores_the_original() {
        for content in [
            format!("#%PAM-1.0\nauth sufficient {HOWDY_MODULE}\nauth include system-login\n"),
            format!(
                "auth\t[success=3 default=ignore]\tpam_fprintd.so\nauth\t[success=2 default=ignore]\t{HOWDY_MODULE}\nauth\t[success=1 default=ignore]\tpam_unix.so\nauth\trequisite\tpam_deny.so\nauth\trequired\tpam_permit.so\n"
            ),
            format!("auth sufficient {HOWDY_MODULE}"),
        ] {
            let new = changed(add_hook(&content, LINE).unwrap());
            assert_eq!(changed(remove_hook(&new).unwrap()), content);
        }
    }

    #[test]
    fn test_remove_keeps_manual_lines() {
        let manual = format!(
            "auth optional pam_exec.so /usr/bin/linux-enable-ir-emitter run\nauth sufficient {HOWDY_MODULE}\n"
        );
        assert_eq!(remove_hook(&manual).unwrap(), Edit::AlreadyDone);
    }

    /// Creates a fake system.
    fn make_system(name: &str) -> System {
        let root = Path::new("target/tmp/setup_howdy").join(name);
        let _ = std::fs::remove_dir_all(&root);
        let pam_dir = root.join("pam.d");
        std::fs::create_dir_all(&pam_dir).unwrap();
        std::fs::write(
            pam_dir.join("sudo"),
            format!("auth sufficient {HOWDY_MODULE}\nauth include system-auth\n"),
        )
        .unwrap();
        std::fs::write(pam_dir.join("passwd"), "password include system-auth\n").unwrap();
        System {
            pam_dir,
            dropin_dir: root.join(format!("etc/systemd/system/{POLKIT_UNIT}.d")),
            reload_systemd: false,
        }
    }

    #[test]
    fn test_setup_and_undo() {
        let system = make_system("setup");
        let exe = Path::new("/usr/bin/linux-enable-ir-emitter");
        let sudo = system.pam_dir.join("sudo");
        let original = std::fs::read_to_string(&sudo).unwrap();
        let dropin = system.dropin_dir.join(POLKIT_DROPIN);

        setup(&system, exe).unwrap();
        assert!(std::fs::read_to_string(&sudo).unwrap().contains(LINE));
        assert_eq!(
            std::fs::read_to_string(&dropin).unwrap(),
            POLKIT_DROPIN_CONTENT
        );
        // the files without howdy are not modified
        assert_eq!(pam_files(&system.pam_dir).unwrap().len(), 2);

        // running it again changes nothing
        setup(&system, exe).unwrap();
        assert_eq!(
            std::fs::read_to_string(&sudo)
                .unwrap()
                .matches(LINE)
                .count(),
            1
        );

        undo_setup(&system).unwrap();
        assert_eq!(std::fs::read_to_string(&sudo).unwrap(), original);
        assert!(!dropin.exists());
        assert!(!system.dropin_dir.exists());
    }

    #[test]
    fn test_setup_without_howdy() {
        let system = make_system("no_howdy");
        std::fs::remove_file(system.pam_dir.join("sudo")).unwrap();
        assert!(setup(&system, Path::new("/usr/bin/linux-enable-ir-emitter")).is_err());
    }

    #[test]
    fn test_setup_refuses_spaces_in_path() {
        let system = make_system("spaces");
        assert!(setup(&system, Path::new("/opt/my apps/linux-enable-ir-emitter")).is_err());
    }
}
