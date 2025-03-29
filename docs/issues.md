## Table of contents
- [Table of contents](#table-of-contents)
- [Please, first, ensure that you use the latest version provided here on GitHub, compare it with `linux-enable-ir-emitter -V`.](#please-first-ensure-that-you-use-the-latest-version-provided-here-on-github-compare-it-with-linux-enable-ir-emitter--v)
- [The configuration has failed](#the-configuration-has-failed)
- [The emitter does not work after successful configuration or after update](#the-emitter-does-not-work-after-successful-configuration-or-after-update)
- [Exception thrown by OpenCV during the configuration](#exception-thrown-by-opencv-during-the-configuration)
- [Other bugs](#other-bugs)

## Please, first, ensure that you use the [latest version provided here on GitHub](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), compare it with `linux-enable-ir-emitter -V`.

## The configuration has failed
If you had the message `INFO: Please shut down your computer, then boot and retry.`, please follow these instructions first:
1. Shutdown your computer, (not just rebooting).
2. Remove the AC adapter and if possible the battery.
3. Wait one minute.
4. Boot.
5. Retry.

Otherwise:
1. But you saw the ir emitter flashing, reboot and switch to manual mode by using the `--manual` option.
2. Also, try the exhaustive search by using the `--limit -1` option (caution: this may take several hours; do not combine it `--manual`).
3. Ensure to configure the right camera by using the option `--device`/
4. Some ir camera requires to specify the resolution, try to use the `--width` and `--height` option. To see what resolutions are available, execute `v4l2-ctl -d /dev/videoX --list-formats-ext` (where `/dev/videoX` is your camera)

Otherwise, open an issue using the "Configuration has failed" template.

## The emitter does not work after successful configuration or after update
1. If you use Howdy, be sure it uses the ir camera configured; the path is printed after the message `INFO: Configuring the camera`.
2. Open an issue using the "Emitter does not work after successful configuration" template.

## Exception thrown by OpenCV during the configuration
1. Add the option `--no-gui`
2. If the issue persist: open an issue using the "Bug report" template, and please build the project manually by following the [readme](../README.md#manual-build) instructions.

## Other bugs
1. If the problem seems to be a library problem, first try the binary I provide by following the [readme](../README.md#installation) instructions. 
2. Open an issue using the "Bug report" template.
