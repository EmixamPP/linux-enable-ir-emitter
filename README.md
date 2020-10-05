# linux-enable-ir-emitter
Provides support for infrared cameras that are not directly supported.

This program was originally designed by PetePriority (https://github.com/PetePriority/chicony-ir-toggle). However, the handling of the error codes was not correct, which made it difficult to modify it.
I also provide in the readme a complete tutorial to help you adapt it to your camera.

This script was created to use Howdy, a Windows Hello for linux.
See <https://github.com/boltgolt/howdy/issues/269> if you want to read the reason why this script was created.

## Setting up the script
1. Download `enable-ir-emitter.c`
2. Run the command `lsusb` to find your camera bus and write it down for later (step 8).
3. List your camera path with `v4l2-ctl --list-devices` and try to find the ir camera with `ffplay /dev/videoX` (ffmpeg package).
When you have found the path corresponding to the ir camera, write it down later (step 12).
4. Install Windows in a VM and allow Windows access to the camera.
5. Install Wireshark on your distro
6. Run the command `sudo modprob usbmon` to allow Wireshark to observe the bus and start it with sudo.
8. In Wireshark and go to the usbmon corresponding to the number of the bus and use the filter `usb.transfer_type == 0x02 && usb.bmRequestType == 0x21`
9. Go in the security options tab of the Windows settings and click on launch an improvement of the recognition of Windows Hello. A pop up is displayed and asks if we want to start the test, you can go back to Wireshark without having to go any further.
10. You can stop the reccording in Wireshark. Now you will have to find the right wValue and wIndex associated with the emitter in Wireshark. For this you will have to test each different proposal of the Wireshark logs.
11. Open the Setup Data section, and note the wValue, the wIndex, the wLength and the Data fragment for later (step 12).
12. Open the `enable-ir-emitter.c` file and modify my information with yours (the file is commented to help you).
13. Run `gcc enable-ir-emitter.c -o enable-ir-emitter` followed by `./enable-ir-emitter`
14. If you don't have an error code, try if it work with `ffplay /dev/videoX` ! (or `sudo howdy test`)
15b. If not go back to step 11 to test the next entry.

For howdy, set the dark_threshold to 100 in `sudo howdy config`

## Launch the script at each startup (with systemd)
1. Download `enable-ir-emitter.service`
2. Copy enable-ir-emitter to /usr/local/bin : `sudo cp enable-ir-emitter /usr/local/bin`
3. Copy `enable-ir-emitter.service` to /etc/systemd/system/ : `sudo cp enable-ir-emitter.service /etc/systemd/system/`
4. Run `sudo systemctl enable enable-ir-emitter && sudo systemctl start enable-ir-emitter` to launch the script each time the system is opened

## Documentation
* <https://www.kernel.org/doc/html/v5.4/media/v4l-drivers/uvcvideo.html>
* <https://wiki.wireshark.org/CaptureSetup/USB>
