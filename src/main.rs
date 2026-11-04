mod configuration;
mod configure;
mod logger;
mod migrate;
mod run;
mod video;

use anyhow::Result;
use clap::{CommandFactory, Parser};

#[tokio::main]
async fn main() -> Result<()> {
    let cli = Cli::parse();

    if cli.config {
        return configuration::print_config();
    }

    if cli.log {
        return logger::print_log();
    }

    if cli.grey_devices {
        return video::print_grey_devices();
    }

    match cli.command {
        Some(Commands::Configure) => {
            logger::init_file()?;
            log::debug!("Version {}.", env!("CARGO_PKG_VERSION"));
            configure::configure().await
        }
        Some(Commands::Run { device, fd, config }) => {
            logger::init_term()?;
            run::run(device.as_deref(), fd, config.as_deref())
        }
        Some(Commands::Migrate { from }) => migrate::migrate(&from),
        None => Ok(Cli::command().print_help()?),
    }
}

#[derive(Parser, Debug)]
#[command(
        version,
        before_help = format!("{} - {}", env!("CARGO_PKG_AUTHORS"), env!("CARGO_PKG_LICENSE")),
        about = "Provides support for infrared cameras.",
        after_help = "https://github.com/EmixamPP/linux-enable-ir-emitter",
        args_conflicts_with_subcommands = true
    )]
struct Cli {
    #[command(subcommand)]
    command: Option<Commands>,
    #[arg(long, help = "Print the configuration file.", conflicts_with_all = ["log", "grey_devices"])]
    config: bool,
    #[arg(
        long,
        help = "Print the recorded logs (for receiving help).",
        conflicts_with = "grey_devices"
    )]
    log: bool,
    #[arg(long, help = "Print the list of grey devices with their UVC controls.")]
    grey_devices: bool,
}

#[derive(clap::Subcommand, Debug)]
enum Commands {
    #[command(about = "Configure your ir camera.")]
    Configure,
    #[command(about = "Apply saved configurations to devices.")]
    Run {
        #[arg(
            short,
            long,
            help = "Specify a device. Default: all configured devices.",
            default_value = None
        )]
        device: Option<std::path::PathBuf>,
        #[arg(
            short,
            long,
            help = "Specify an opened file descriptor (in RDWR) for the device. Default: open a new one.",
            default_value = None,
            requires = "device"
        )]
        fd: Option<i32>,
        #[arg(
            short,
            long,
            help = "Specify the configuration file to use. Default: use the default configuration path.",
            default_value = None,
        )]
        config: Option<std::path::PathBuf>,
    },
    #[command(
        about = "Migrate the v6 configurations to the v7 format. The configured devices must be connected."
    )]
    Migrate {
        #[arg(
            short,
            long,
            help = "Directory of the v6 configurations.",
            default_value = migrate::V6_CONFIG_DIR
        )]
        from: std::path::PathBuf,
    },
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cli_is_valid() {
        Cli::command().debug_assert();
    }

    #[test]
    fn test_cli_print_options_conflict() {
        assert!(Cli::try_parse_from(["prog", "--config", "--log"]).is_err());
        assert!(Cli::try_parse_from(["prog", "--log", "--grey-devices"]).is_err());
        assert!(Cli::try_parse_from(["prog", "--config", "--grey-devices"]).is_err());
    }

    #[test]
    fn test_cli_print_options_conflict_with_commands() {
        assert!(Cli::try_parse_from(["prog", "--config", "run"]).is_err());
        assert!(Cli::try_parse_from(["prog", "--log", "configure"]).is_err());
    }

    #[test]
    fn test_cli_migrate() {
        match Cli::try_parse_from(["prog", "migrate"]).unwrap().command {
            Some(Commands::Migrate { from }) => {
                assert_eq!(from, std::path::Path::new(migrate::V6_CONFIG_DIR))
            }
            other => panic!("unexpected command: {other:?}"),
        }
        match Cli::try_parse_from(["prog", "migrate", "--from", "/opt/v6"])
            .unwrap()
            .command
        {
            Some(Commands::Migrate { from }) => assert_eq!(from, std::path::Path::new("/opt/v6")),
            other => panic!("unexpected command: {other:?}"),
        }
    }

    #[test]
    fn test_cli_single_options() {
        assert!(Cli::try_parse_from(["prog", "--config"]).unwrap().config);
        assert!(Cli::try_parse_from(["prog", "--log"]).unwrap().log);
        assert!(
            Cli::try_parse_from(["prog", "--grey-devices"])
                .unwrap()
                .grey_devices
        );
        assert!(matches!(
            Cli::try_parse_from(["prog", "run", "--config", "/tmp/conf.toml"])
                .unwrap()
                .command,
            Some(Commands::Run { .. })
        ));
    }
}
