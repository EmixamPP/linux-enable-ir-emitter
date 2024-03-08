## System requirements
* if you used chicony-ir-toggle, please remove it
* `python3 >= 3.10`
* `video4linux` as video capture devices framework
* `systemd` or `openrc` as service manager
* for the tarball artifact provided on GitHub: `glibc`, `libstdc++`, `libgcc`, `gtk3`

## Manual build requirements
### Libs:
* `opencv devel`
* `gtk3 devel`
* `yaml-cpp devel`
### Tools:
* `meson >= 1.0.0`
* `ninja-build`
* `g++` (for `C++17`)

## Hardware requirements
* An infrared camera with at least one emitter.
* A camera compatible with the UVC standard. Typically, it has to be a USB camera (laptop camera are mostly USB).