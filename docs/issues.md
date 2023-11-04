## Please, first, ensure that you have the last version. Compare the [latest release](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest) with `linux-enable-ir-emitter -V`

## The configuration has failed 
If you had the message `INFO: Please shut down your computer, then boot and retry.`, please follow these instructions first:
1. Shutdown your computer, (not just rebooting)
2. Remove the AC adapter and if possible the battery
3. Wait one minute
4. Boot
5. Retry

If the configuration failed:
1. if you saw the ir emitter flashing, reboot and switch to manual mode by using the `-m` option.
2. Also, try the exhaustive search by using the `-l -1` option (caution: this may take several hours; do not combine it `-m`).

Otherwise, open an issue using the "Configuration has failed" template.

## The emitter does not work after successful configuration or after update.
1. If you use Howdy, be sure it uses the infrared camera configured; the path is printed after the message `INFO: Configuring the camera:`.
2. Execute `sudo linux-enable-ir-emitter boot enable`.
3. Open an issue using the "Emitter does not work after successful configuration" template.

## Error with the systemd/openrc or udev boot service
1. Ensure you have `systemd` or `openrc` (and the related version installed)
2. Open an issue using the "Emitter does not work after successful configuration" template.

## Exception thrown by OpenCV during the configuration
1. Add the option `-g`
2. If the issue persist: open an issue using the "Bug report" template, and please build the project manually by following the [readme](../README.md#manual-build) instructions.

## Other bugs
1. If the problem seems to be a library problem, first try the binary I provide by following the [readme](../README.md#installation) instructions. 
2. Open an issue using the "Bug report" template.
