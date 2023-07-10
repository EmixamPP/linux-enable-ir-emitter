## System requirements
* if you used chicony-ir-toggle, please remove it
* `python3 >= 3.10`
* `video4linux` as video capture devices framework
* `systemd` or `openrc` as service manager
* `glibc`, `libstdc++`, `libgcc`

## Manual build requirements
* `meson >= 0.63.0`, `ninja-build`, `pkg-config`
* `cmake` only if you build OpenCV
* `g++`, `libstdc++-devel` (for `C++20`)
* `curl`

## Hardware requirements
* An infrared camera with at least one emitter.
* A camera compatible with the UVC standard. Typically, it has to be a USB camera (laptop camera are mostly USB).