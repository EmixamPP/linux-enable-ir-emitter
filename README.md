# linux-enable-ir-emitter
Provides support for infrared cameras that are not directly supported.

This code is based on that of PetePriority <https://github.com/PetePriority/chicony-ir-toggle>. 
I brought a better debugging and I will provide you a complete tutorial in the readme to adapt it to your camera.

This script was created to use Howdy, a Windows Hello for linux.
See <https://github.com/boltgolt/howdy/issues/269> if you want to read the reason why this script was created.

## Setting up the script
1. Download `enable-ir-emitter.c`
2. Run the command `lsusb` to find your camera bus and write it down for later (step 5).
3. List your camera path with `v4l2-ctl --list-devices` and try to find the ir camera with `ffplay /dev/videoX` (ffmpeg package).
When you have found the path corresponding to the ir camera, write it down later (step 9).
4. Install Windows in a VM (VMware Player or VirtualBox) and allow Windows access to the camera
5. Install Wireshark on your distro and start it with sudo
6. Go to the usbmon corresponding to the number of the bus
7. Now, launch an improvement of the recognition of Windows Hello in the security options tab of the settings.
8. Back to Wireshark and find a packet with the info "URB_CONTROL out" (you can sort by info to go faster). 
   Wireshark will show you at the bottom 3 sections: Frame, USB URB, Setup Data.
9. Open the Setup Data section, and note the wValue, the wIndex and the Data (or Data fragment) for later (step 9).
10. Open the `enable-ir-emitter.c` file and modify my information with yours.
11. Run `gcc enable-ir-emitter.c -o enable-ir-emitter` followed by `./enable-ir-emitter`
(12. For howdy, set thedark_threshold to 100 in `sudo howdy config`)
If you do not get an error message, go and test to see if it worked !

## Launch the script at each startup
1. Download `enable-ir-emitter.service`
2. Copy enable-ir-emitter to /opt : `sudo cp enable-ir-emitter /opt`
3. Copy `enable-ir-emitter.service` to /etc/systemd/system/ : `sudo cp enable-ir-emitter.service /etc/systemd/system/`
4. Run `sudo systemctl enable enable-ir-emitter && sudo systemctl start enable-ir-emitter` to launch the script each time the system is opened

## Documentation
* <https://www.kernel.org/doc/html/v4.12/media/v4l-drivers/uvcvideo.html>
* <https://wiki.wireshark.org/CaptureSetup/USB>
