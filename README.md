# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically generate a lightweight driver (located in user space) for almost any (UVC) infrared emitter.

This tool was created to use [Howdy](https://github.com/boltgolt/howdy), a Windows Hello for linux.

## Installation
Directly refer to the manual buid [section](#manual-build) if you have an ARM architecture or your boot service manager is not Systemd but OpenRC. Stay here if you don't know what all this means.
 
Download the latest `linux-enable-ir-emitter-x.x.x.tar.gz` [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:
```
sudo tar -C / --no-same-owner -h -xzf linux-enable-ir-emitter-*.tar.gz
```

It can be uninstalled by executing:
```
sudo rm -rf /usr/lib64/linux-enable-ir-emitter \
/etc/linux-enable-ir-emitter \
/usr/libexec/linux-enable-ir-emitter \
/usr/bin/linux-enable-ir-emitter \
/usr/lib/systemd/system/linux-enable-ir-emitter.service \
/etc/udev/rules.d/99-linux-enable-ir-emitter.rules \
/usr/share/bash-completition/completitions/linux-enable-ir-emitter
```

## How to enable your infrared emitter?
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

## Manual build
See [wiki](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Requirements) for specification concerning build requirements.

Clone the git:
```
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
```

Download a tiny version of opencv that will be statically linked. If you are under ARM architecture, just replace `x86-64` by `aarch64` in the url. This is not required, you can use the shared opencv library of your distro. But it is recommanded in order to do not have issues after distro updates:
```
curl -L https://github.com/EmixamPP/opencv-tiny/releases/download/4.7.0/opencv-tiny-4.7.0_x86-64.tar.gz | tar -xJ
sed -i sed -i "s|=\(opencv-tiny-4.7.0\)|=${PWD}/\1|g" opencv-tiny-4.7.0/lib/pkgconfig/{opencv4.pc,zlib.pc} 
```

Setup build (remove `--pkg-config-path...` if you skipped the previous step):
```
meson setup build --pkg-config-path opencv-4.7.0/lib/pkgconfig
```

Only if you do not have Systemd but OpenRC:
```
meson configure build -Dboot_service=openrc
```

Build and install linux-enable-ir-emitter:
```
sudo meson install -C build
```

You can uninstall the software by executing `sudo ninja uninstall -C build`. 
