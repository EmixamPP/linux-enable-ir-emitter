## Manual build
See [docs/requirements.md](docs/requirements.md) for specifications and dependencies concerning build requirements.

Feel free to open an issue if you need help!

Setup build:
```
git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
cd linux-enable-ir-emitter
meson setup build
```

If you do not have Systemd but OpenRC:
```
meson configure build -Dboot_service=openrc
```

You plan to contribute? Enable the debug build:
```
meson configure build --buildtype debug
```

For immutable distro,
you may want to change to installation directory,
by default /usr/local, to $HOME/.local:
```
meson configure build --prefix=$HOME/.local
```

If you do not want to rebuild the tool after each update of your distro,
you may want to statically link the dependencies (this requires also static build of the libs):
```
meson configure build --prefer-static
```

Compile and install:
```
meson compile -C build
meson install -C build
```

You can uninstall the tool by executing `sudo ninja uninstall -C build` (thus do not remove the build directory). 
