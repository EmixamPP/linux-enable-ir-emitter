# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically configure any infrared camera. 

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
<a href="https://copr.fedorainfracloud.org/coprs/emixampp/linux-enable-ir-emitter/"><img src="https://copr.fedorainfracloud.org/coprs/emixampp/linux-enable-ir-emitter/package/linux-enable-ir-emitter/status_image/last_build.png"></a> Page link : [COPR package](https://copr.fedorainfracloud.org/coprs/emixampp/linux-enable-ir-emitter/).
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

## How to enable your infrared emitter ?

0. `linux-enable-ir-emitter -h`
1. `sudo linux-enable-ir-emitter configure` look closely at the ir emitter and answer to the asked questions. You can specify your infrared camera with the option `-d /dev/videoX`, by default is /dev/video2
2. `sudo linux-enable-ir-emitter boot enable`

If this doesn't work for you, you can always try to configure your infrared camera yourself by following the tutorial on [this wiki page](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration).

## Issues
Please consult the [wiki](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki) before open an issue. It contains a lot of useful information !

## :hearts: Thanks to
[@cchvuth](https://github.com/cchvuth) [@Diaoul](https://github.com/Diaoul) [@FabioLolix](https://github.com/FabioLolix) [@furcelay](https://github.com/furcelay) [@komex](https://github.com/komex) [@m4rtins](https://github.com/m4rtins) [@renyuneyun](https://github.com/renyuneyun)  [@reolat](https://github.com/reolat) [@Stkai](https://github.com/Stkai) [@supdrewin](https://github.com/supdrewin) 
