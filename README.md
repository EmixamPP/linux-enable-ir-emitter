# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically generate a lightweight driver (located in user space) for almost any (UVC) infrared emitter.

This software was created to use [Howdy](https://github.com/boltgolt/howdy), a Windows Hello for linux.

## Installation
ARM architecture are supported, refer directly to the manual build section. Distributions repository and package are no longer supported.
For more information, please read this [post](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/About-distributions-repository). 
 
Download the latest `linux-enable-ir-emitter-x.x.x.tar.gz` archive [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:
```
sudo tar -C / -h -xzf linux-enable-ir-emitter-*.tar.gz
# if you are under Fedora or any system with SELinux
sudo sh fix_SELinux.sh apply
```

It can be uninstalled by executing:
```
sudo rm -r /usr/lib64/linux-enable-ir-emitter
sudo rm -r /etc/linux-enable-ir-emitter
sudo rm /usr/bin/linux-enable-ir-emitter
sudo rm /usr/lib/systemd/system/linux-enable-ir-emitter.service
sudo rm -f /etc/udev/rules.d/99-linux-enable-ir-emitter.rules
sudo rm /usr/share/bash-completition/completitions/linux-enable-ir-emitter
```

### Manual build :
The following tools are needed (see [wiki](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Requirements) for further specification) : meson, cmake
```
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter

# build a tiny version of opencv that will be statically linked
# not required, you can use the shared opencv library of your distro
# but recommanded in order to do not have issues after distro updates
sh build_opencv.sh

# build linux-enable-ir-emitter
# remove the option --pkg-config-path ... if you have not built opencv
meson setup build --pkg-config-path opencv-*/build/install_dir/lib*/pkgconfig
sudo meson install -C build

# if you are under Fedora or any system with SELinux
sudo sh fix_SELinux.sh apply
```
You can uninstall the software by executing `sudo ninja uninstall -C build`. 

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
