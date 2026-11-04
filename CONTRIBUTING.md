# Contributing to linux-enable-ir-emitter
Thank you for considering contributing to our project! We appreciate your interest and effort in improving linux-enable-ir-emitter. Please follow the guidelines below to ensure a smooth contribution process.

## Packaging Guidelines
When building a package for distribution, please ensure the following:
1. You can modify the configuration and log file paths in [.cargo/config.toml](.cargo/config.toml) to match the packaging standards of the target distribution. Note that the defaults allow rootless utilization.
2. The only external *compile time* dependency needed are
   * `gcc`
   * `libclang`
   * the Linux kernel headers (`linux/videodev2.h`), used to generate the V4L bindings. They are usually already installed with the C toolchain (e.g. `linux-libc-dev` on Debian/Ubuntu, `linux-api-headers` on Arch, `kernel-headers` on Fedora).
3. Build the project using:
    ```
   cargo build --release
    ```
   The resulting binary will be located at `target/release/linux-enable-ir-emitter`. You can also use `cargo install --path <...>` to your convenience.

   To cross-compile for ARM 64-bit (aarch64) from x86-64, you additionally need the aarch64 cross toolchain and its headers, and to point the bindings generator to these headers (this is only needed for cross-compiling, not when building natively on an ARM computer). For example on Debian/Ubuntu:
    ```
   sudo apt-get install gcc-aarch64-linux-gnu libc6-dev-arm64-cross
   rustup target add aarch64-unknown-linux-gnu
   BINDGEN_EXTRA_CLANG_ARGS_aarch64_unknown_linux_gnu=-I/usr/aarch64-linux-gnu/include \
       cargo build --release --target aarch64-unknown-linux-gnu
    ```
   The linker is already configured in [.cargo/config.toml](.cargo/config.toml), and the resulting binary will be located at `target/aarch64-unknown-linux-gnu/release/linux-enable-ir-emitter`.
4. The v7 is incompatible with the v6. If applicable, please make sure that your users migrate their saved configuration with the `linux-enable-ir-emitter migrate` command (see the [README](README.md#migration-from-v6)).
   * The command must be executed by each user, since the v7 configuration is saved by default in the home directory, and the cameras must be connected. So it is better to inform the users (e.g. with a post-install message) than to execute it in a post-install script running as root.
   * If your v6 package used another `config_dir` than `/etc`, the users have to pass `--from <config_dir>/linux-enable-ir-emitter`.
   * The v6 configurations are only read, so you can keep `/etc/linux-enable-ir-emitter/` until your users have migrated.

## Contributing Code
This project is using the usual Rust conventions. Here are some additional explanations:

1. Build with:
    ```
   cargo build
    ```
   The resulting binary will be located at `target/debug/linux-enable-ir-emitter`
   > [!NOTE]
   > With a debug build, any camera can be used, even not infrared ones. This is useful for development and testing.
2. Run with:
    ```
    cargo run -- <your arguments>
    ```
3. Add the pre-commit hooks to make sure the linting checks and tests are passing before each commit:
   ```
   cargo install cargo-insta cargo-shear --locked
   git config core.hooksPath .githooks
   ```
   Additionally, it will automatically format/fix the issues for you when possible.
4. The UI is tested with snapshots. Run the tests with `cargo insta test --unreferenced=reject`, which also fails if a snapshot is not used by any test anymore (e.g. after renaming a test), like the CI does. If you modified the UI intentionally, you will need to review and accept the new snapshots:
   ```
   cargo insta review
   ```
5. Please make sure to write tests for your code changes where applicable.
6. Commit, push and please describe enough what you did in your PR description. Thank you very much!
