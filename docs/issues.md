### Please, first, ensure that you have the last version. Compare the [latest release](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest) with `linux-enable-ir-emitter -V`

## The configuration has failed 
If you had the message `INFO: Please shut down your computer, then boot and retry.`, please follow these instructions first:
1. Shutdown your computer, (not just rebooting)
2. Remove the AC adapter and if possible the battery
3. Wait one minute
4. Boot
5. Retry

Otherwise, open an issue using the "Configuration has failed" template.

## The emitter does not work after successful configuration or after update.
1. If you use Howdy, be sure it uses the infrared camera configured; the path is printed after the message `INFO: Configuring the camera:`.
2. Execute `sudo linux-enable-ir-emitter boot enable`.
3. If none of the previous fix your problem, open an issue using the "Emitter does not work after successful configuration" template.

## Error with the systemd/openrc or udev boot service
1. Ensure you have `systemd` or `openrc` (and the related version installed)
2. Open an issue using the "Emitter does not work after successful configuration" template.

## Other bugs
Open an issue using the "Bug report" template.
