## Manual build
You plan to contribute? Read [CONTRIBUTING.md](../CONTRIBUTING.md) instead.

Feel free to open an issue if you need help!

1. Clone the project:

    ```
    git clone https://github.com/EmixamPP/linux-enable-ir-emitter.git
    cd linux-enable-ir-emitter
    ```

2. Setup the paths:

    Adjust the [config.toml](../.cargo/config.toml) file to set where to:
    * install the executable
    * write the config file
    * write the log file

3. Build and install

   ```
   cargo install --path .
   ```

If you want to uninstall, you can check [uninstallation.md](uninstallation.md).