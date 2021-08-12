# linux-enable-ir-emitter
Provides support for infrared cameras that are not directly supported (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called. 

The original script was designed by [@PetePriority](https://github.com/PetePriority/chicony-ir-toggle). However, the handling of the error codes was not correct, which made it difficult to modify it and no explanation was given. (Only one infrared camera is supported)\
At first I just wrote a tutorial and now I have made it into a utility that allows you to configure any infrared camera almost automatically !

This script was created to use Howdy, a Windows Hello for linux <https://github.com/boltgolt/howdy>.

## Installation
### For Arch distro based :
`linux-enable-ir-emitter` is available as an [AUR package](https://aur.archlinux.org/packages/linux-enable-ir-emitter/).
A VCS variant is also available as an [git AUR package](https://aur.archlinux.org/packages/linux-enable-ir-emitter-git/).
``` shell
git clone https://aur.archlinux.org/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
makepkg -csi

linux-enable-ir-emitter -h
```

### For other distro :
``` shell
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
sudo bash installer.sh install

linux-enable-ir-emitter -h
```

You can easily uninstall the software by executing `sudo bash installer.sh uninstall`.

#### Optional installation :
If you need to use the `linux-enable-ir-emitter full` command, please read the related [wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration).\
This command is not always necessary, so I have removed the dependencies from the standard installation (feel free to let me know what you think about that). 

## How to enable your infrared emitter ?
1. Try the quick configuration, it is the easiest and does not require any manipulation : `sudo linux-enable-ir-emitter quick`.
2. If this does not work, you will have to undertake a semi-automatic configuration with `sudo linux-enable-ir-emitter full`.
This requires more preparation, please follow [the wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration). 
3. To enable the infrared emitter at boot, simply run `sudo linux-enable-ir-emitter boot enable`.

If this doesn't work for you, you can always try to configure your infrared camera yourself by following the tutorial on [this wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration).

## Issues
- If you had used `chicony-ir-toggle` before, and when opening the system, the systemd service won't start: use the installer script to clean the installation: `sudo bash installer.sh repair`. Which will uninstall `chicony-ir-toggle` and install `linux-enable-ir-emitter`. 
- At startup, if you have the error `Unable to open a file descriptor for /dev/videoX`. Take a look at issue [#1](https://github.com/EmixamPP/linux-enable-ir-emitter/issues/1).

## :hearts: Thanks to
For helping me to improve the software:\
[@m4rtins](https://github.com/m4rtins) [@supdrewin](https://github.com/supdrewin) [@renyuneyun](https://github.com/renyuneyun) [@furcelay](https://github.com/furcelay)

For managing its distribution:\
[@Diaoul](https://github.com/Diaoul) [@komex](https://github.com/komex) [@FabioLolix](https://github.com/FabioLolix)

For sharing their camera configuration:\
[@Stkai](https://github.com/Stkai) [@reolat](https://github.com/reolat) [@gregor160300](https://github.com/gregor160300)
