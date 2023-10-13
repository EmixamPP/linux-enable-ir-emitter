# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically generate a lightweight driver (located in user space) for almost any (UVC) infrared emitter.

## Installation
Download the latest `linux-enable-ir-emitter-x.x.x.systemd.x86-64.tar.gz` [here](https://github.com/EmixamPP/linux-enable-ir-emitter/releases/latest), then execute:
```
sudo tar -C / --no-same-owner -h -xzf linux-enable-ir-emitter*.tar.gz
```

We also support OpenRC, just download the `.openrc` variant.

### Uninstallation
It can be uninstalled by executing (remove the last line to keep the emitter configuration):
```
sudo rm -rf /usr/lib64/linux-enable-ir-emitter \
/usr/libexec/linux-enable-ir-emitter \
/usr/bin/linux-enable-ir-emitter \
/usr/lib/systemd/system/linux-enable-ir-emitter.service \
/etc/udev/rules.d/99-linux-enable-ir-emitter.rules \
/usr/share/bash-completition/completitions/linux-enable-ir-emitter \
/etc/linux-enable-ir-emitter
```

## How to enable your infrared emitter?
1. Stand in front of and close to the camera and make sure the room is well lit.
2. Ensure to not use the camera during the execution.
3. Be patient, do not kill the process.
4. Execute `sudo linux-enable-ir-emitter configure`.
    * If you have many emitters on the camera, specify it using the option `-e`. E.g. `-e 2` if you have 2 emitters.
5. Answer to the asked questions.
6. Sometimes, it can request you to shut down, then boot and retry ($\neq$ reboot)

If you like the project, do not hesitate to star the repository to support me, thank you!

Any criticims, ideas and contributions are welcome.

If the configuration failed:
1. But you saw the ir emitter flashing, reboot and switch to manual mode by using the `-m` option.
2. Also, try the exhaustive search by using the `-l -1` option (caution: this may take several hours; do not combine it `-m`).
3. Otherwise, feel free to open an issue, *but please consult the [docs](docs/README.md) first*.

The software supports the configuration of multiple devices, execute the configure command and specify each time which device to configure.

## Manual build
See [docs](docs/requirements.md) for specification concerning build requirements.

Setup build:
```
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
meson setup build
```

Only if you do not have Systemd but OpenRC:
```
meson configure build -Dboot_service=openrc
```

Compile and install:
```
meson compile -C build
meson install -C build
```

You can uninstall the tool by executing `sudo ninja uninstall -C build`. 
