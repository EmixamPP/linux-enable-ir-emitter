# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically generate a lightweight driver (located in user space) for almost any (UVC) infrared emitter.

This tool was created to use [Howdy](https://github.com/boltgolt/howdy), a Windows Hello for linux.

## Installation
ARM architecture are supported, refer directly to the manual build section. Distributions repository and package are no longer supported.
For more information, please read this [post](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/About-distributions-repository). 
 
Download the latest `linux-enable-ir-emitter-x.x.x.tar.gz` archive [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:
```
sudo tar -C / --no-same-owner -h -xzf linux-enable-ir-emitter-*.tar.gz
```

If you are under Fedora or any system with SELinux, also execute: 
```
semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/bin/execute-driver
semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/bin/driver-generator
restorecon -v /usr/lib64/linux-enable-ir-emitter/bin/*
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

Clone the git:
```
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
```

Download a tiny version of opencv that will be statically linked. If you are on ARM plateform, you have to build it yourself by executing the [script here](https://github.com/EmixamPP/opencv-tiny/blob/main/build_opencv.sh). This is not required, you can use the shared opencv library of your distro. But it is recommanded in order to do not have issues after distro updates:
```
curl -L https://github.com/EmixamPP/opencv-tiny/raw/main/opencv-4.7.0.tar.xz | tar -xJ
sed -i "3s@^prefix=.*@prefix=${PWD}/opencv-4.7.0@" opencv-4.7.0/lib64/pkgconfig/opencv4.pc 
```

Build linux-enable-ir-emitter (remove `--pkg-config-path...` if you skipped the previous step, or change the path by `opencv-*/build/install_dir/lib*/pkgconfig` if you built it yourself):
```
meson setup build --pkg-config-path opencv-4.7.0/lib64/pkgconfig
sudo meson install -C build
```

If you are under Fedora or any system with SELinux, also execute: 
```
semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/bin/execute-driver
semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/bin/driver-generator
restorecon -v /usr/lib64/linux-enable-ir-emitter/bin/*
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
