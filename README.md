# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can almost automatically, configure any infrared camera. 

This software was created to use Howdy, a Windows Hello for linux <https://github.com/boltgolt/howdy>.

## Installation
### For Arch distro based : 
<a href="https://aur.archlinux.org/packages/linux-enable-ir-emitter/"><img src="https://img.shields.io/aur/version/linux-enable-ir-emitter"></a> Page links : [Release AUR package](https://aur.archlinux.org/packages/linux-enable-ir-emitter/) and [VCS AUR package](https://aur.archlinux.org/packages/linux-enable-ir-emitter-git/).
``` shell
git clone https://aur.archlinux.org/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
makepkg -csi
``` 

### For Fedora distro based (Mageia + openSUSE) : 
<a href="https://copr.fedorainfracloud.org/coprs/emixampp/linux-enable-ir-emitter/"><img src="https://img.shields.io/badge/copr-v2.1.0--1-blue"> <img src="https://copr.fedorainfracloud.org/coprs/emixampp/linux-enable-ir-emitter/package/linux-enable-ir-emitter/status_image/last_build.png"></a> Page link : [COPR package](https://copr.fedorainfracloud.org/coprs/emixampp/linux-enable-ir-emitter/).
``` shell
sudo dnf copr enable emixampp/linux-enable-ir-emitter
sudo dnf --refresh install linux-enable-ir-emitter
```

### For other distro :
<a href="https://github.com/emixampp/linux-enable-ir-emitter/releases"><img src="https://img.shields.io/github/release/emixampp/linux-enable-ir-emitter.svg?colorB=4c1"></a>

``` shell
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
sudo bash installer.sh install
```

You can easily uninstall the software by executing `sudo bash installer.sh uninstall`.

#### Optional installation :
If you need to use the `linux-enable-ir-emitter full` command, please read the related [wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration).\
This command is not always necessary, so I have removed the dependencies from the standard installation (feel free to let me know what you think about that). 

## How to enable your infrared emitter ?

0. `linux-enable-ir-emitter -h`
1. Try the quick configuration, it is the easiest and does not require any manipulation : `sudo linux-enable-ir-emitter quick`.
2. If this does not work, you will have to undertake a semi-automatic configuration with `sudo linux-enable-ir-emitter full`.
This requires more preparation, please follow [the wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration). 
3. To enable the infrared emitter at boot, simply run `sudo linux-enable-ir-emitter boot enable`.

If this doesn't work for you, you can always try to configure your infrared camera yourself by following the tutorial on [this wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration).

## Issues
- If you had used `chicony-ir-toggle` before: please execute `sudo linux-enable-ir-emitter fix chicony`
- If the config file is corrupted: execute `sudo linux-enable-ir-emitter fix config`

## :hearts: Thanks to
For helping me to improve the software:\
[@m4rtins](https://github.com/m4rtins) [@supdrewin](https://github.com/supdrewin) [@renyuneyun](https://github.com/renyuneyun) [@furcelay](https://github.com/furcelay) [@cchvuth](https://github.com/cchvuth)

For managing its distribution:\
[@Diaoul](https://github.com/Diaoul) [@komex](https://github.com/komex) [@FabioLolix](https://github.com/FabioLolix)

For sharing their camera configuration:\
[@Stkai](https://github.com/Stkai) [@reolat](https://github.com/reolat) [@gregor160300](https://github.com/gregor160300) [@kapi117](https://github.com/kapi117)

## Documentation 
* [chicony-ir-toggle - MIT licence](https://github.com/PetePriority/chicony-ir-toggle).
* <https://www.kernel.org/doc/html/v5.14/userspace-api/media/drivers/uvcvideo.html>
* <https://wiki.wireshark.org/CaptureSetup/USB>
