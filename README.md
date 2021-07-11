# linux-enable-ir-emitter
Provides support for infrared cameras that are not directly supported (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called. 

The original script was designed by [@PetePriority](https://github.com/PetePriority/chicony-ir-toggle). However, the handling of the error codes was not correct, which made it difficult to modify it and no explanation was given.\
At first I just wrote a tutorial and now I have made it into a utility that allows you to configure any infrared camera almost automatically !

This script was created to use Howdy, a Windows Hello for linux <https://github.com/boltgolt/howdy>.

## Installation
You don't necessarily have to install my utility software, it can be used directly from the cloned repertory. But you won't have access to all the features. (Only the "quick" and "run" commands will work) 

Please install the dependency wireshark :
  - `sudo pacman -S wireshark-cli` for Arch distro based 
  - `sudo dnf install wireshark-cli` for Fedora distro based 
  - `sudo apt install tshark` for Debian distro based 
  -  otherwise install `wireshark` and it should work in any case.

Then, let's install linux-enable-ir-emitter :
``` shell
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
sudo bash installer.sh install

linux-enable-ir-emitter -h
```

## How to enable your infrared emitter ?
1. Try the quick configuration, it is the easiest and does not require any manipulation : `linux-enable-ir-emitter quick`.
2. If this does not work, you will have to undertake a semi-automatic configuration with `sudo linux-enable-ir-emitter full`.
This requires more preparation, please follow [the wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration). 
3. To enable the infrared emitter at boot, simply run `linux-enable-ir-emitter boot enable`

If this doesn't work for you, you can always try to configure your infrared camera yourself by following the tutorial on [this wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration).

## Issues
- If you had used `chicony-ir-toggle` before, and when opening the system, the systemd service won't start: use the installer script to clean the installation: `sudo bash installer.sh repair`. Which will uninstall `chicony-ir-toggle` and reinstall `linux-enable-ir-emitter`. 
- At startup, if you have the error `Unable to open a file descriptor for /dev/videoX`. Take a look at issue [#1](https://github.com/EmixamPP/linux-enable-ir-emitter/issues/1).

## :hearts: Thanks to
for helping me to improve the software:\
[@renyuneyun](https://github.com/renyuneyun)  [@supdrewin](https://github.com/supdrewin)  [@m4rtins](https://github.com/m4rtins)


for sharing their camera setup with the community to improve the automatic setup:\
[@Stkai](https://github.com/Stkai) [@reolat](https://github.com/reolat) [@gregor160300](https://github.com/gregor160300)
