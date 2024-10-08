# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognize your infrared camera). The purpose of this repository is to enable the emitter when the infrared camera is invoked.

`linux-enable-ir-emitter` can automatically configure almost any (UVC) infrared emitter.

## Installation
Download the latest [linux-enable-ir-emitter-x.x.x.systemd.x86-64.tar.gz](https://github.com/EmixamPP/linux-enable-ir-emitter/releases). Then execute:
```
sudo tar -C / --no-same-owner -m -h -vxzf linux-enable-ir-emitter*.tar.gz
sudo systemctl enable --now linux-enable-ir-emitter
```

We also support the OpenRC service manager. See [docs/manual-build.md](docs/manual-build.md) for information on how to build the project.

To uninstall the tool, see [docs/uninstallation.md](docs/uninstallation.md) for the instructions.

## How do I enable my infrared emitter?
1. Stand in front of and close to the camera and make sure the room is well lit.
2. Make sure you are not using the camera during the execution.
3. Be patient, do not kill the process (you could break the camera); it is not stuck if you see the `Searching...` output blinking. If you really have to, press `CTRL+C`, the tool will stop safely as soon as possible (you may have to answer a last question see 5.).
4. Run `sudo linux-enable-ir-emitter configure`.
    * If you have many emitters on the camera, specify it with the option `--emitters`. E.g. `--emitters 2`.
    * If your ir camera requires a specific resolution, specify it with the option `--width` and `--height`. E.g. `--width 640 --height 360`.
    * The tool should automatically detect your ir camera, but you can specify it with the option `--device`. E.g. `--device /dev/video2`; useful if you have multiple ir camera.
5. You will see a video feedback, answer to the asked questions by pressing Y or N inside the window (click on it to give the focus if it is not the case).
6. Sometimes, it can request you to shut down, then boot and retry ($\neq$ reboot)

If you like the project, do not hesitate to star the repository to support me, thank you!

If the configuration failed:
1. But you saw the ir emitter flashing, reboot and switch to manual mode by with the `--manual` option.
2. Also, try the exhaustive search by using the `--limit -1` option (caution: this may take several hours; do not combine it `--manual`).
3. Otherwise, feel free to open a new issue, **but please consult the [docs](docs/README.md) first**.

All criticism, ideas and contributions are welcome!

## How do I tweak my camera?
Some cameras provide instructions for changing the brightness of the ir emitter.
You will need to find the corresponding instructions and set the correct value manually.
An instruction consists of X values between 0-255.

1. Run `sudo linux-enable-ir-emitter tweak`
   * If you ir camera requires a specific resolution, specify it with the option `--width` and `--height`. E.g. `--width 640 --height 360`.
   * The tool should detect automatically your ir camera, but you can specify it with the option `--device`. E.g. `--device /dev/video2`; useful if you have multiple ir camera.
2. You will see a video feedback and a menu of the available instructions.
3. By default, some instructions may be marked as `[DISABLE]`, which means that they may make your camera unusable, so the tool will not use them automatically.
4. Select one, then the status (enable/disable), the initial and current value for the instructions, as well as the minimum and maximum values (if exists) will be displayed.
5. Input a new value and watch the video feedback to see the difference. You can also input `enable` or `disable` to change its status.
6. If you made your camera unusable, it can request you to shut down, then boot and retry ($\neq$ reboot). You may also want to reset everything to default, for that see [docs/configurations](docs/configurations).
