# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [7.0.0-beta]
### Changed
- Complete rewrite in Rust, bringing more robustness and performance.
- New TUI interface for improved user experience and understanding.
- `tweak` command is now accessible via the `configure` command.
- Better balance between configuration search time and exhaustiveness.

### Added
- More debug commands to help users to report issues: `--config`, `--log`, `--grey-devices`.
- Migration script for the configuration file from v6 to v7 format.

### Fixed
- Use `/dev/v4l/by-id/` device path instead of `/dev/v4l/by-path/` under the hood for better persistence on some system/hardware.

### Removed
- OpenCV is not used anymore for capturing V4L video stream, because of unreliability, and compatibility issues across distro.
- Removing `test` command.

## [6.1.2] - 2025-09-01
### Added
- Add custom meson options: `config_dir`, `create_config_dir`, `create_log_dir`

## [6.1.1] - 2025-01-29
### Fixed
- The `run` command does not crash if one configuration cannot be applied on q device.

## [6.1.0] - 2025-01-29
### Added
- Unit tests for instruction manipulations.
- New `meson test` target including: unit tests + `clang-format` + `clang-tidy`.

### Modified
- The user no longer has to type in the video feedback, but in the terminal.
- Better configuration handling.
- Better instruction search.
- Better error handling.
- Static linking of `libgcc` and `libstdc++` if `--prefer-static` `meson` option is enabled.
- `disable` configuration field is deprecated and renamed `status` for more flexibility. It accepts the strings: `start`, `idle`, `disable`.
- The `run` command only applies instructions with the `start` `status`.
- The `tweak` command displays all the time the video feedback.
- Bump OpenCV to v4.11.0
- Bump argparse to v3.2

### Removed
- `--werror` `meson` option no longer enabled by default.
- `spdlog` dependency not needed anymore.

### Fixed
- `OpenCV` error: `size.width>0 && size.height>0 in function 'imshow'`.

## [6.0.6] - 2024-10-11

### Fixed
- Improved building system compatibility.

## [6.0.5] - 2024-10-02

### Fixed
- Compatibility issue with `fmt` library.

## [6.0.4] - 2024-09-30

### Fixed
- Crash in `tweak` command.

## [6.0.3] - 2024-09-01

### Fixed
- Inconsistent file logging.
- Missing help print.

### Changed
- Minor general improvements.

## [6.0.0] - 2024-06-14

### Added
- YAML format support for storing camera configurations.
- Ability to save all camera instructions.
- Manual tweaking of camera instructions.
- Option to specify camera resolution.
- Zsh completion.
- Logging in file.
- Catch `ctrl-c` to prevent camera interruptions.

### Fixed
- Freeze in video feedback.
- Issue with `test` command.
- Handling of multiple v4l paths for the same device.

### Removed
- Boot command.
- Python code completely migrated to C++.

### Changed
- Reduced installation size.
- Enhanced logging system.

## [5.2.4] - 2023-11-01

### Fixed
- Config generated but not found issue.
- Exception thrown by `OpenCV`.
- Reduced installation size.

## [5.2.1] - 2023-10-20

### Fixed
- Unable to execute commands.

## [5.2.0] - 2023-10-19

### Added
- New `test` command.

### Changed
- Improved efficiency.
- Significantly reduced installation size.

## [5.0.4] - 2023-10-09

### Added
- New advice and hint messages.

## [5.0.2] - 2023-08-23

### Fixed
- Minor search issue.

### Changed
- Default longer search duration.

## [5.0.0] - 2023-08-08

### Added
- Automatic IR camera detection.
- Automatic IR emitter configuration.
- Exhaustive search functionality.
- Video feedback feature.

## [4.8.2] - 2023-06-23

### Fixed
- Occasional systemd hanging issue.
- Improved installation paths.

## [4.8.0] - 2023-06-21

### Added
- Native SELinux compatibility.

## [4.7.0] - 2023-06-20

### Added
- `OpenRC` support.

### Changed
- Requirement for Python version >= 3.10.

## [4.5.0] - 2023-02-26

### Changed
- Config generation.

## [4.4.2] - 2023-02-24

### Fixed
- Command not found error.

### Changed
- Reduced size.

## [4.4.0] - 2023-02-17

### Added
- Support for multiple emitter cameras.
- Persistent usage of `/dev/v4l/by-path`.

### Fixed
- Memorization of broken instructions for skipping.

### Changed
- Complete rework of implementation.

## [4.1.5] - 2022-09-13

### Fixed
- Boot service for custom devices.

## [4.1.4] - 2022-08-11

### Added
- Forced V4L2 backend in `OpenCV`.

### Changed
- Config generation.

## [4.1.2] - 2022-07-04

### Added
- Asynchronous camera triggering.

### Fixed
- Camera triggering issue.
- Device symlink boot service side effect.

## [4.0.0] - 2022-06-19

### Changed
- Config generation through rework, optimization, and enhancement.

### Removed
- Manual configuration commands.
- Integration option for Howdy.

## [3.2.5] - 2021-12-09

### Added
- Bash auto-completion.
- Better systemd support.

### Changed
- Integration with [Howdy](https://github.com/boltgolt/howdy).

## [3.2.2] - 2021-11-04

### Added
- Support for any device path format.

### Changed
- Systemd service.

## [3.2.0] - 2021-10-23

### Added
- Support for multiple devices.

## [3.1.1] - 2021-09-23

### Added
- Limit for repeated negative answers to the same pattern.

## [3.1.0] - 2021-09-22

### Added
- New configuration system.
- Exit codes for better error handling.

### Changed
- Configuration file location.

## [2.1.0] - 2021-08-29

### Added
- New `fix` command to resolve common issues.

### Fixed
- Systemd service file to prevent `/dev/video` file descriptor error.

## [2.0.1] - 2021-08-12

### Initial Release
