# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically generate a lightweight driver (located in user space) for any (UVC) infrared emitter.

This software was created to use [Howdy](https://github.com/boltgolt/howdy), a Windows Hello for linux.

## Installation
ARM architecture are supported, refer directly to the manual build section. 

Distributions repository are no longer supported, and will never be supported again.
For more information, please read this [post](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/About-distributions-repository). 
### rpm package :  
Download the rpm package [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:
``` shell
sudo rpm -iv linux-enable-ir-emitter-*.rpm
```

### deb package : 
Download the deb package [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:
``` shell
sudo dpkg -i linux-enable-ir-emitter-*.deb
```

### Arch distro based : 
``` shell
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter/arch
makepkg -csi
``` 

### Manual build :
The following dependencies are needed (see [wiki](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Issues#requirements) for further specification) : Meson, Python, OpenCV C++ libraries
``` shell
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
meson setup build
sudo meson install -C build

# if you are under Fedora are any system with SELinux
sudo shell fix_SELinux.sh apply
```
You can uninstall the software by executing `sudo ninja uninstall -C build`. 
Or `sudo find / -name "linux-enable-ir-emitter" -exec rm -r "{}" \;`.

## How to enable your infrared emitter ?
1. Ensure to not use the camera during the execution.
2. Be patient, do not kill the process, and whatever the reason. (Unless the execution is stuck for more than 10 minutes.)
3. Execute `sudo linux-enable-ir-emitter configure`.
    * You can specify your infrared camera with the option `-d`, by default it is `-d /dev/video2`.
    * If you have many emitters on the camera, specify it using the option `-e`. E.g. `-e 2` if you have 2 emitters.
4. Answer to the asked questions.
5. Sometimes, it can request you to shut down, then boot and retry ($\neq$ reboot)

If you like the project, do not hesitate to star the repository to support me, thank you !

*Please consult the [wiki](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki) before opening an issue.*

The software supports the configuration of multiple devices, execute the configure command and specify each time which device to configure.

## :hearts: Thanks to
[@Bakunin-san](https://github.com/Bakunin-san) [@cchvuth](https://github.com/cchvuth) [@CharlesNRU](https://github.com/CharlesNRU) [@Diaoul](https://github.com/Diaoul) [@Eeems](https://github.com/Eeems) [@FabioLolix](https://github.com/FabioLolix) [@furcelay](https://github.com/furcelay) [@komex](https://github.com/komex) [@logicito](https://github.com/logicito) [@m4rtins](https://github.com/m4rtins) [@renyuneyun](https://github.com/renyuneyun) [@reolat](https://github.com/reolat) [@Stkai](https://github.com/Stkai) [@supdrewin](https://github.com/supdrewin)

for having reported a bug, tested the version in development, contributed to the code or helped to maintain the distributions package.
