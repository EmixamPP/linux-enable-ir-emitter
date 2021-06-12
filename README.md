# linux-enable-ir-emitter
Provides support for infrared cameras that are not directly supported (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called. 

This program was originally designed by [@PetePriority](https://github.com/PetePriority/chicony-ir-toggle). However, the handling of the error codes was not correct, which made it difficult to modify it. And the way it is written makes it quite complicated to understand. I also provide in the readme a complete tutorial to help you adapt it to your camera. As well as an automatic setup script. 

This script was created to use Howdy, a Windows Hello for linux <https://github.com/boltgolt/howdy>.

## Automatic setup
When the community uses my git and follows the steps explained below, it happens that some share the script configuration that works with their camera. 
In order to help those who are less comfortable with this kind of manipulation, I have set up an automatic script that tests on your camera the different configurations that users have shared. 
So before you start reading my whole tutorial below, try the automatic setup first, if it works you just saved at least an hour! 

1. Run the python script `cd auto && python auto-config.py`
2. The script will ask you to test your infrared camera, you just have to answer with yes/no if the infrared emitter works
3. If no configuration works, go to the Setting up the script section bellow.
4. If a configuration works, it will propose you to create a systemd service to automatically activate the transmitter at system startup. 

You can easily uninstall what the script did with: `bash cleaner uninstall` (thanks to [@supdrewin](https://github.com/supdrewin))

## Setting up the script
(For information, the values used in the script are the one that works for my Lenovo T15.)
Before starting all these steps; some users have shared their script settings. Take a look in `config.txt`, if you find your computer (or perhaps a similar reference), you can go directly to step 11 and modify the lines referenced by `config.txt`.

1. Open the `manual` directory and open `enable-ir-emitter.c` in your text editor.
2. Run the command `lsusb` to find your camera bus and write it down for later (step 7).
3. List your camera path with `v4l2-ctl --list-devices` and try to find the ir camera, e.g. with `ffplay /dev/videoX` (ffmpeg package).
When you have found the path corresponding to the ir camera, write it down later (step 11).
4. Install Windows in a VM and allow Windows access to the camera.
5. Install Wireshark on your distro
6. Run the command `sudo modprobe usbmon` to allow Wireshark to observe the bus and start it with sudo : `sudo wireshark`.
8. In Wireshark, go to the usbmon corresponding to the number of the bus. After that, use the filter `usb.transfer_type == 0x02 && usb.bmRequestType == 0x21` (to only display camera control packets)
9. In the VM, go in the security options tab of the Windows settings and click on launch an improvement of the recognition of Windows Hello. A pop up is displayed and asks if we want to start the test, you can go back to Wireshark without having to go any further.
10. You can stop the reccording in Wireshark. Now you will have to find the right wValue and wIndex associated with the emitter in Wireshark. For this you will have to test each different proposal of the Wireshark logs.
11. Open the Setup Data section, and note the wValue, the wIndex, the wLength and the Data fragment for the next step.
12. In the `enable-ir-emitter.c` file : modify my information with yours (the file is commented to help you).
13. Disconnect the camera from the vm and run `gcc enable-ir-emitter.c -o enable-ir-emitter` followed by `./enable-ir-emitter`
14. If you don't have an error code, try if it work with `ffplay /dev/videoX` ! (or `sudo howdy test`)
15. If not go back to step 11 to test the next entry.

For howdy, set the dark_threshold to 100 in `sudo howdy config`

## Launch the script at each startup (with systemd)
1. Download enable-ir-emitter.service.
2. Copy enable-ir-emitter to /usr/local/bin : `sudo cp enable-ir-emitter /usr/local/bin`.
3. Copy `enable-ir-emitter.service` to /etc/systemd/system/ : `sudo cp enable-ir-emitter.service /etc/systemd/system/`.
4. Run `sudo systemctl enable --now enable-ir-emitter` to launch the script each time the system is opened.

## Issues
- If you had used `chicony-ir-toggle` before, and when opening the system, the systemd service won't start: use the [@supdrewin](https://github.com/supdrewin) script to clean the installation: `bash cleaner reinstall`. Which will uninstall `chicony-ir-toggle` and run my automatic installation script. 
- At startup, if you have the error `Unable to open a file descriptor for /dev/videoX` [@m4rtins](https://github.com/m4rtins) found a solution for this problem. Take a look at issue [#1](https://github.com/EmixamPP/linux-enable-ir-emitter/issues/1).

## Note
However, at the step 8-9, to find the packet that activates the emitter may be sent at another time. For example, it may be sent at the beginning of the test, so you have to launch it. Or at the moment when Windows start because with my camera Windows turns off the transmitter every time it is no longer needed but maybe not with your camera.

I can therefore only advise you to test as many cases as possible where the emitter could receive a packet asking it to prepare to emit when the camera is turned on. I remain available if you need more details !

## Documentation
* <https://www.kernel.org/doc/html/v5.12/userspace-api/media/drivers/uvcvideo.html>
* <https://wiki.wireshark.org/CaptureSetup/USB>
