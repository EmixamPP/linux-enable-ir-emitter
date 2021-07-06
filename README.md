# linux-enable-ir-emitter
Provides support for infrared cameras that are not directly supported (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called. 

This program was originally designed by [@PetePriority](https://github.com/PetePriority/chicony-ir-toggle). However, the handling of the error codes was not correct, which made it difficult to modify it.\
At first I just wrote a tutorial and now I have made it into a utility that allows you to configure any infrared camera almost automatically !

This script was created to use Howdy, a Windows Hello for linux <https://github.com/boltgolt/howdy>.

## How to enable your infrared emitter ?
You don't necessarily have to install my utility software, it can be used directly from the cloned repertory. But you won't have access to all the features. 

1. To begin, please install the dépendency :
  - `sudo pacman -S wireshark-cli` for Arch distro based 
  - `sudo dnf install wireshark-cli` for Fedora distro based 
  - `sudo apt install tshark` for Debian distro based 
  -  otherwise install `wireshark` and it should work in any case.

2. Then, let's install the utility :
``` shell
git clone -b software https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
sudo bash installer.sh install
```
3. First of all, try the quick configuration, it is the easiest and does not require any manipulation : `sudo linux-enable-ir-emitter quick`.
4. The utility will guide you through the terminal to ask you questions and find out if it worked.
5. If not, you will have to undertake a semi-automatic configuration with `sudo linux-enable-ir-emitter full`.
This requires more preparation, but don't worry [a wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration) and the terminal are there to guide you. 
6. To enable the infrared emitter at boot, simply run `sudo linux-enable-ir-emitter boot enable`

## Issues
- If you had used `chicony-ir-toggle` before, and when opening the system, the systemd service won't start: use the installer script to clean the installation: `sudo bash installer.sh repair`. Which will uninstall `chicony-ir-toggle` and reinstall `linux-enable-ir-emitter`. 
- At startup, if you have the error `Unable to open a file descriptor for /dev/videoX`. Take a look at issue [#1](https://github.com/EmixamPP/linux-enable-ir-emitter/issues/1).

## Thanks to
For helping me to improve the software:\
[@renyuneyun](https://github.com/renyuneyun)  [@supdrewin](https://github.com/supdrewin)  [@m4rtins](https://github.com/m4rtins)


For sharing their camera setup with the community to improve the automatic setup:\
[@Stkai](https://github.com/Stkai) [@reolat](https://github.com/reolat) [@gregor160300](https://github.com/gregor160300)
