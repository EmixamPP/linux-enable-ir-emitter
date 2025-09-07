# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box (at the very least, the kernel must recognize your infrared camera). The purpose of this repository is to enable the emitter when the infrared camera is invoked.

`linux-enable-ir-emitter` can automatically configure almost any (UVC) infrared emitter.

## Installation
Download the latest [linux-enable-ir-emitter-x.x.x.systemd.x86-64.tar.gz](https://github.com/EmixamPP/linux-enable-ir-emitter/releases). Then execute:
```
sudo tar -C / --no-same-owner -m -h -vxzf linux-enable-ir-emitter*.tar.gz
```

To uninstall the tool, see [docs/uninstallation.md](docs/uninstallation.md) for the instructions.

### Integration with Howdy
In all file returned by `grep -rl howdy /etc/pam.d`, add the following line before the one mentioning "howdy":
```
auth optional pam_exec.so /usr/local/bin/linux-enable-ir-emitter run
```

Note that the path to the binary may vary depending on your installation method. You can determine the correct path by running `which linux-enable-ir-emitter`.

### Integration with other program
For version 6.x.x, we still provide a systemd service that, for most cameras, should be sufficient to keep the ir emitter enabled at all times:

```
sudo systemctl enable --now linux-enable-ir-emitter
```
Note that this approach is now deprecated and support will be removed from version 7.x.x.

However, this approach is more of a workaround than a proper solution. Therefore, if your goal is to use Howdy, please refer to the previous section. Otherwise, you will need to find a way to run the command `linux-enable-ir-emitter run` before using the camera in your program.

We also support the OpenRC service manager. See [docs/manual-build.md](docs/manual-build.md) for information on how to build the project.

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
2. Also, try the exhaustive search by using the `--limit -1` option. Caution: this may take several hours; do not combine it `--manual`. Put something that reflects IR in front of the computer so you can leave.
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
3. Beside of each instruction, you will see the status of the instruction:
   * If marked `[DISABLE]`, the instruction may make your camera unusable, so the tool will not use them automatically, we advise you do not touch it.
   * If marked `[START]`, the instruction will be executed on the `linux-enable-ir-emitter run` command; and by extension when the systemd service is started (at boot).
   * If marked `[IDLE]`, the instruction will not be executed on the `linux-enable-ir-emitter run` command.
4. Select one, then the status (start/idle/disable), the initial and current value for the instructions, as well as the minimum and maximum values (if exists) will be displayed.
5. Input a new value and watch the video feedback to see the difference. You can also input a different status.
6. You may also want to reset everything to default, for that see [docs/configurations](docs/configurations).
