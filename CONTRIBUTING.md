# Contributing to linux-enable-ir-emitter
Thank you for considering contributing to our project! We appreciate your interest and effort in improving linux-enable-ir-emitter. Please follow the guidelines below to ensure a smooth contribution process.

## Packaging Guidelines
When building a package for distribution, please ensure the following:
1. You can modify the configuration and log file paths in [.cargo/config.toml](.cargo/config.toml) to match the packaging standards of the target distribution. Note that the defaults allow rootless utilization.
2. The only external *compile time* dependency needed are
   * `gcc`
   * `libclang`
3. Build the project using:
    ```
   cargo build --release
    ```
   The resulting binary will be located at `target/release/linux-enable-ir-emitter`. You can also use `cargo install --path <...>` to your convenience.
4. The v7 is incompatible with the v6. If applicable, please make sure to use the provided [migration script]() on the saved configuration.
   > [!Important]
   > This script is not yet available. It will be provided when the v7 will be officially released (currently in beta).

## Contributing Code
This project is using the usual Rust conventions. Here are some additional explanations:

1. Build with:
    ```
   cargo build
    ```
   The resulting binary will be located at `target/debug/linux-enable-ir-emitter`

   > [!NOTE]
   > With a debug build, any camera can be used, even not infrared ones. This is useful for development and testing.

2. Add the pre-commit hooks to make sure the linting checks and tests are passing before each commit:
   ```
   git config core.hooksPath .githooks
   ```
   Additionally, it will automatically format/fix the issues for you when possible.
3. If you modified the UI intentionally, you will need to re-generate the UI test snapshots:
   ```
   cargo install cargo-insta
   cargo insta review
   ```
4. Please make sure to write tests for your code changes where applicable.
5. Commit, push and please describe enough what you did in your PR description. Thank you very much!
