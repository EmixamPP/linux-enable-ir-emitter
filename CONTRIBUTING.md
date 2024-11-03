# Contributing to linux-enable-ir-emitter
Thank you for considering contributing to our project! We appreciate your interest and effort in improving linux-enable-ir-emitter. Please follow the guidelines below to ensure a smooth contribution process.

## Reporting Issues
Before opening a new issue, please check the [docs](docs/README.md) first in order to either find answer to your question or to use the right template.

## Contributing Code
To be sure that the modifications you made to the code will pass the CI during the PR, please perform the following steps:
1. Compile locally the dependencies:
    ```
    cd .github/workflows/deps && cmake -Bbuild -GNinja && ninja -C build && cd -
    ```
2. Setup the build:
    ```
    meson setup build --buildtype=debug --sysconfdir /etc --prefer-static --pkg-config-path=$(find . -name "pkgconfig")
    ```
3. Perform changea and compile:
   ```
   meson compile −C build
   ```
   your new version of `linux-enable-ir-emitter` is located in `build/src/`
4. Ensure tests pass:
   ```
   meson test -C build
   ```

   If `clang-format` fails, you can easily fix the errors with:
   ```
   ninja clang-format -C build
   ```
5. Commit, push and please describe enough what you did in your PR description
