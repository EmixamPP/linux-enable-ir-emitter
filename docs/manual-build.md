## Manual build
See [docs/requirements.md](docs/requirements.md) for specifications and dependencies concerning build requirements.

Feel free to open an issue if you need help!

1. Clone the project:
    ```
    git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
    cd linux-enable-ir-emitter
    ```

2. You want to build the project using static libraries?\
    This is recommanded to avoid breaking the tool after distro update, because they are directly included inside the generated executable.
    We provide a CMake file to easily build all the dependencies. Npthing will be installed on your system:
    ```
    cd .github/workflows/deps
    cmake . -B build -GNinja
    ninja -C build
    cd -
    ```

3. Setup the build:
    * If you built the dependecies using the previous step:
    ```
    meson setup build --prefer-static --pkg-config-path=$(find .github -name "pkgconfig")
    ```
    * Otherwise:
    ```
    meson setup build
    ```

4. If you do not have Systemd but OpenRC:
    ```
    meson configure build -Dboot_service=openrc
    ```

5. You plan to contribute? Enable the debug build:
    ```
    meson configure build --buildtype debug
    ```

6. Compile and install:
    ```
    meson compile -C build
    meson install -C build
    ```

You can uninstall the tool by executing `sudo ninja uninstall -C build` (thus do not remove the build directory) or see [docs/uninstallation](uninstallation.md). 
