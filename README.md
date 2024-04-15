# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognise your infrared camera). The purpose of this repository is to activate the emitter when the infrared camera is called.

`linux-enable-ir-emitter` can automatically configure almost any (UVC) infrared emitter.

## Installation
Download the latest [linux-enable-ir-emitter-x.x.x.systemd.x86-64.tar.gz](https://github.com/EmixamPP/linux-enable-ir-emitter/releases). Then execute:
```
sudo tar -C / -vxzf linux-enable-ir-emitter*.tar.gz
sudo systemctl enable linux-enable-ir-emitter
```

We also support the OpenRC service manager. See [docs/manual-build.md](docs/manual-build.md) for information on how to build the project.

To uninstall the tool, see [docs/uninstallation.md](docs/uninstallation.md) for the instructions.

## How to enable your infrared emitter?
1. Stand in front of and close to the camera and make sure the room is well lit.
2. Ensure to not use the camera during the execution.
3. Be patient, do not kill the process.
4. Execute `sudo linux-enable-ir-emitter configure`.
    * If you have many emitters on the camera, specify it using the option `--emitters`. E.g. `--emitters 2`.
    * If you ir camera requires a specific resolution, specify it using the option `--width` and `--height`. E.g. `--width 640 --height 360`.
    * The tool should detect automatically your ir camera, but you can specify it using the option `--device`. E.g. `--device /dev/video2`; useful if you have multiple ir camera.
5. You will see a video feedback, answer to the asked questions by pressing Y or N inside the window.
6. Sometimes, it can request you to shut down, then boot and retry ($\neq$ reboot)

If you like the project, do not hesitate to star the repository to support me, thank you!

If the configuration failed:
1. But you saw the ir emitter flashing, reboot and switch to manual mode by using the `--manual` option.
2. Also, try the exhaustive search by using the `--limit -1` option (caution: this may take several hours; do not combine it `--manual`).
3. Otherwise, feel free to open an issue, **but please consult the [docs](docs/README.md) first**.

Any criticims, ideas and contributions are welcome!

## How to tweak your camera?
Some cameras provide instrutions for changing the brightness of the ir emiter.
You will need to find the corresponding instructions and the correct value manually.
An instruction consists of X values between 0-255.

1. Execute `sudo linux-enable-ir-emitter tweak`
   * If you ir camera requires a specific resolution, specify it using the option `--width` and `--height`. E.g. `--width 640 --height 360`.
   * The tool should detect automatically your ir camera, but you can specify it using the option `--device`. E.g. `--device /dev/video2`; useful if you have multiple ir camera.
2. You will see a video feedback and a menu of the available instructions.
3. Select one, then the initial and current value for the instructions, as well as the minimum and maximum values (if exists) are displayed.
4. Input a new value and observe the video feedback to see the difference.
5. If you made your camera unusable, it can request you to shut down, then boot and retry ($\neq$ reboot)
