## System requirements
* if you used chicony-ir-toggle, please remove it
* `video4linux` as video capture devices framework
* for the tarball artifact provided on GitHub: `libstdc++ >= 13.0.0`, `libgcc >= 13.0.0`, `gtk3`, `glibc`

## Manual build requirements
## Building tools:
* `gcc >= 13.0.0`
* `meson >= 1.0.0`
* `ninja-build`
### C++ libs:
* `opencv devel`
* `gtk3 devel`
* `yaml-cpp devel`
* `argparse devel`
* `spdlog devel`
* if meson flag `-Dtests=true`: `gest-devel`

## Hardware requirements
* An infrared camera with at least one emitter.
* A camera compatible with the UVC standard. Typically, it has to be a USB camera (laptop camera are mostly USB).