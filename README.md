# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically generate a lightweight driver (located in user space) for any (UVC) infrared emitter.

This software was created to use [Howdy](https://github.com/boltgolt/howdy), a Windows Hello for linux.

## Installation
ARM architecture are supported, refer directly to the manual build section. 

Distributions repository are no longer supported.
For more information, please read this [post](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/About-distributions-repository). 
### rpm package :  
Download the rpm package [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:
```
sudo rpm -iv --nodeps linux-enable-ir-emitter-*.rpm
```

### deb package : 
~~Download the deb package [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:~~ Temporarily unavaible, it will be fixed soon
```
sudo dpkg -i linux-enable-ir-emitter-*.deb
```

### Arch distro based : 
```
mkdir linux-enable-ir-emitter && cd linux-enable-ir-emitter
curl -O https://raw.githubusercontent.com/EmixamPP/linux-enable-ir-emitter/master/packages/arch/PKGBUILD
curl -O https://raw.githubusercontent.com/EmixamPP/linux-enable-ir-emitter/master/packages/arch/linux-enable-ir-emitter.install
makepkg -csi
``` 

### Manual build :
The following dependencies are needed (see [wiki](https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Requirements) for further specification) : python, meson, cmake
```
# clone the git
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter

# build a minimal version of opencv that will be staticaly linked 
# not required you can use the shared opencv library package of your distro
# but recommanded in order to do not have issues after distro updates
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.7.0.zip
unzip opencv.zip
mkdir -p opencv-4.7.0/build && cd opencv-4.7.0/build
cmake .. -DBUILD_SHARED_LIBS=OFF -DBUILD_LIST=videoio -DOPENCV_GENERATE_PKGCONFIG=YES -DCMAKE_INSTALL_PREFIX=./tmp_install
cmake --build .
make install
cd ../../

# build linux-enable-ir-emitter
# remove the option --pkg-config-path if you didn't build opencv
meson setup build --pkg-config-path opencv-4.7.0/build/tmp_install/lib*/pkgconfig
sudo meson install -C build

# if you are under Fedora or any system with SELinux
sudo shell fix_SELinux.sh apply

# clean all
# if perhaps you wish to uninstall it,
# just save the linux-enable-ir-emitter/build directory
cd ../ && rm -r linux-enable-ir-emitter
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
