# linux-enable-ir-emitter
Provides support for infrared cameras that are not directly supported.

This script was created to use Howdy, a Windows Hello for linux.
See <https://github.com/boltgolt/howdy/issues/269> if you want to read the reason why this script was created.

## Setting up the script
1. Download `enable-ir-emitter.c`
2. Run the command `lsusb` to find your camera bus and write it down for later (step 8).
3. List your camera path with `v4l2-ctl --list-devices` and try to find the ir camera with `ffplay /dev/videoX` (ffmpeg package).
When you have found the path corresponding to the ir camera, write it down later (step 14).
4. Install Windows in a VM and allow Windows access to the camera.
5. Install Wireshark on your distro
6. Run the command `sudo modprob usbmon` to allow Wireshark to observe the bus and start it with sudo.
8. In Wireshark and go to the usbmon corresponding to the number of the bus and use the filter `usb.transfer_type == 0x02 && usb.bmRequestType == 0x21`
9. Go in the security options tab of the Windows settings and click on launch an improvement of the recognition of Windows Hello. A pop up is displayed and asks if we want to start the test, you can leave without having to go any further.
10. Back to Wireshark and stop the recording (you can stop recording at the start of the Windows Hello test to reduce the number of logs in wireshark).
12. You can stop the reccording in Wireshark. Now you will have to find the right wValue and wIndex associated with the emitter in Wireshark. For this you will have to test each different proposal of the Wireshark logs.
13. Open the Setup Data section, and note the wValue, the wIndex, the wLength and the Data fragment for later (step 14).
14. Open the `enable-ir-emitter.c` file and modify my information with yours (the file is commented to help you).
15. Run `gcc enable-ir-emitter.c -o enable-ir-emitter` followed by `./enable-ir-emitter`
16. Try if it work with `ffplay /dev/videoX` ! (or `sudo howdy test`)
16b. If not go back to step 13 to test the next entry.

For howdy, set thedark_threshold to 100 in `sudo howdy config`

## Launch the script at each startup (with systemd)
1. Download `enable-ir-emitter.service`
2. Copy enable-ir-emitter to /usr/local/bin : `sudo cp enable-ir-emitter /usr/local/bin`
3. Copy `enable-ir-emitter.service` to /etc/systemd/system/ : `sudo cp enable-ir-emitter.service /etc/systemd/system/`
4. Run `sudo systemctl enable enable-ir-emitter && sudo systemctl start enable-ir-emitter` to launch the script each time the system is opened

## Documentation
* <https://www.kernel.org/doc/html/v4.12/media/v4l-drivers/uvcvideo.html>
* <https://wiki.wireshark.org/CaptureSetup/USB>
