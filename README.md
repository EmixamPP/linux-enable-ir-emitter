# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically generate a lightweight driver (located in user space) for any (UVC) infrared emitter.

This software was created to use [Howdy](https://github.com/boltgolt/howdy), a Windows Hello for linux.

## Installation
### For Fedora distro based (.rpm) :  
Page link : [COPR package](https://copr.fedorainfracloud.org/coprs/emixampp/linux-enable-ir-emitter/).
``` shell
sudo dnf copr enable emixampp/linux-enable-ir-emitter
sudo dnf --refresh install linux-enable-ir-emitter
```

### For Arch distro based : 
Page links : [Release AUR package](https://aur.archlinux.org/packages/linux-enable-ir-emitter/) and [VCS AUR package](https://aur.archlinux.org/packages/linux-enable-ir-emitter-git/).
``` shell
git clone https://aur.archlinux.org/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
makepkg -csi
``` 

### For Debian distro based (.deb) : 
Page link : [PPA package](https://launchpad.net/~emixampp/+archive/ubuntu/linux-enable-ir-emitter).
``` shell
sudo add-apt-repository ppa:emixampp/linux-enable-ir-emitter
sudo apt update
sudo apt install linux-enable-ir-emitter
```

### For other distro :
The following dependencies are needed : python3, opencv (C++)
``` shell
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
sudo bash installer.sh install
```
You can easily uninstall the software by executing `sudo bash installer.sh uninstall`. Or update it with `sudo bash installer.sh update`.

## How to enable your infrared emitter ?
1. Ensure to not use the camera during the execution.
2. Be patient, do not kill the process, and whatever the reason.
3. Execute `sudo linux-enable-ir-emitter configure`. You can specify your infrared camera with the option `-d /dev/videoX`, by default it is /dev/video2.
4. Answer to the asked questions.

If you like the project, do not hesitate to star the repository to support me, thank you !

Please consult the [wiki](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki) before open an issue.

The software supports the configuration of multiple devices, execute the configure command and specify each time which device to configure.

## :hearts: Thanks to
[@cchvuth](https://github.com/cchvuth) [@Diaoul](https://github.com/Diaoul) [@FabioLolix](https://github.com/FabioLolix) [@furcelay](https://github.com/furcelay) [@komex](https://github.com/komex) [@logicito](https://github.com/logicito) [@m4rtins](https://github.com/m4rtins) [@renyuneyun](https://github.com/renyuneyun) [@reolat](https://github.com/reolat) [@Stkai](https://github.com/Stkai) [@supdrewin](https://github.com/supdrewin) 

for having reported a bug, tested the version in development, contributed to the code or helped to maintain the distributions package.