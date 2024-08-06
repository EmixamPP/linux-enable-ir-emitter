## System requirements
* if you used chicony-ir-toggle, please remove it
* `video4linux` as video capture devices framework
* for the tarball artifact provided on GitHub: `glibc`, `libstdc++`, `libgcc`, `gtk3`

## Manual build requirements
### C/C++ libs:
* `opencv devel`
* `gtk3 devel`
* `yaml-cpp devel`
* `argparse devel`
* `spdlog devel`
### Tools:
* `meson >= 1.0.0`
* `ninja-build`
* `gcc >= 10.0.0`

## Hardware requirements
* An infrared camera with at least one emitter.
* A camera compatible with the UVC standard. Typically, it has to be a USB camera (laptop camera are mostly USB).