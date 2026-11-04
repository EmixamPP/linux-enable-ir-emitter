# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box on Linux (at the very least, the kernel must recognize your infrared camera). The purpose of this repository is to enable the emitter when the infrared camera is invoked.

`linux-enable-ir-emitter` can automatically configure almost any UVC infrared camera.

> [!NOTE]
> This tool drives **UVC** infrared cameras. On some newer laptops, the IR sensor is behind a vendor-specific IPU protocol.
>
> To tell which kind you have:
>
> ```bash
> linux-enable-ir-emitter --grey-devices  # your camera listed -> this tool applies
> # Alternatively if linux-enable-ir-emitter is not yet installed:
> ls /sys/bus/usb/drivers/uvcvideo/       # your camera listed -> this tool applies
> v4l2-ctl --list-devices | grep -A2 ipu  # an IPU device shown -> it does not (v4l2-ctl is provided by the v4l-utils package)
> ```
>
> <details><summary>Projects for IPU cameras:</summary>
>
> * [svp7500-camera-fix-pack](https://github.com/jibsta210/svp7500-camera-fix-pack) for the Himax HM1092 behind a Synaptics SVP7500 bridge
>
></details>

> [!IMPORTANT]
> Please read the documentation below carefully. It can save you a lot of time and help you successfully enable your infrared camera.

> [!NOTE]
> If you plan to package this software for a Linux distribution, or to contribute code, please read [CONTRIBUTING.md](CONTRIBUTING.md).

## Installation

Download the latest asset from the [releases](https://github.com/EmixamPP/linux-enable-ir-emitter/releases), depending on your processor architecture (`uname -m`):

* `linux-enable-ir-emitter-x.x.x-release-x86-64.tar.gz` for Intel and AMD processors (most computers)
* `linux-enable-ir-emitter-x.x.x-release-aarch64.tar.gz` for ARM 64-bit processors

Then execute:

```sh
sudo tar -C /usr/local/bin/ --no-same-owner -m -vxzf linux-enable-ir-emitter*.tar.gz
```

The installation consists of 3 files:

* The executable: `/usr/local/bin/linux-enable-ir-emitter`
* The logs (for receiving help): `$HOME/.local/state/linux-enable-ir-emitter.log`, you can print them with `linux-enable-ir-emitter --log`. Once bigger than 1 MiB, they are moved to `linux-enable-ir-emitter.log.old`.
* The camera configuration (can be backed up): `$HOME/.config/linux-enable-ir-emitter.toml`, you can print it with `linux-enable-ir-emitter --config`

### Migration from v6

```sh
   linux-enable-ir-emitter migrate
```

<details><summary>Click for more details</summary>

The configuration format of the v7 is not compatible with the v6 one. If you used the v6, you can migrate your configuration:

1. Install the v7 (see above), but keep the v6 configuration directory `/etc/linux-enable-ir-emitter/`.
2. Make sure your camera is connected and not used, then execute as your normal user, **not with sudo**, since the v7 configuration is saved in your home directory:

   ```sh
   linux-enable-ir-emitter migrate
   ```

   * Only the controls applied by the v6 are migrated. The controls disabled in the v6 are blacklisted, so the *IR Enabler* will not modify them.
   * If the v6 configurations are located elsewhere (e.g. installed by a distro package), add `--from <DIRECTORY>`.
   * You can safely run it again: a camera that already has a v7 configuration is skipped.

3. Check the result with `linux-enable-ir-emitter --config`.
4. Uninstall the v6 by following its [uninstallation instructions](https://github.com/EmixamPP/linux-enable-ir-emitter/blob/6.1.2/docs/uninstallation.md). The v7 does not use a boot service anymore, see the integrations below.

</details>

### Integration with Howdy

Once howdy is installed and configured, execute:

```sh
sudo linux-enable-ir-emitter setup-howdy
```

<details><summary>Click for more details</summary>
The following line is added before howdy, in all the files of `/etc/pam.d/`.

  ```txt
  auth optional pam_exec.so quiet <path_to_bin>/linux-enable-ir-emitter run
  ```

* Since polkit 126, password prompts are executed in a sandbox without access to the camera nor to the home directories. Therefore, a systemd drop-in is added to allow access to the video devices, and the home directories in read-only: `/etc/systemd/system/polkit-agent-helper@.service.d/linux-enable-ir-emitter.conf` containing:

```ini
[Service]
PrivateDevices=no
DeviceAllow=char-video4linux rw
ProtectHome=read-only
```

You can remove all the changes with `sudo linux-enable-ir-emitter setup-howdy --undo`.

</details>

### Integration with other program

You will need to execute the `linux-enable-ir-emitter run` command before the program that uses the infrared camera.

Alternatively, if you can and/or want to integrate better with the program that uses the camera, you can pass an opened file descriptor for the camera to the command: `linux-enable-ir-emitter run --device <DEVICE> --fd <FD>`.

> [!IMPORTANT]
> You will need to pass the config path as argument to `linux-enable-ir-emitter run --config <CONFIG_PATH>` **when executed as root outside PAM**, if `linux-enable-ir-emitter configure` was executed as a normal user. When executed by PAM, the configuration of the user who is authenticating is used automatically.

## How do I enable my infrared emitter?

0. For a better experience, use a large terminal window.
1. Execute the command: `linux-enable-ir-emitter configure`
2. Select the *IR Enabler* tool. It will iterate through the UVC camera controls and modify them to try to find the one that enables the IR emitter.
3. (Optional) Configure your *Device settings*:
   * *Path*: by default, a grey camera will be selected (if any). If you have multiple cameras, you can change the path. To know which ones are grey cameras, execute `linux-enable-ir-emitter --grey-devices`.
   * *Number of emitters*: if your camera has multiple IR emitters, specify it here (at least 1).
   * *Resolution height/width*: by default, the camera driver will select a resolution for you. You can change it if needed.
   * *FPS*: by default, the camera driver will select a frame rate for you. You can change it if needed.
4. (Optional) Configure the *Search settings*:
   * *Manual validation*: if enabled, our stream analyzer will not be used to detect if the IR emitter is working. Instead, you will have to confirm manually using the camera preview or the IR emitter itself. **Enable this mode if the configuration failed the first time.**
   * *Images to analyze in auto validation*: number of images used by the stream analyzer to decide if the IR emitter is working. A larger value can lead to more reliable results, but will take more time. Increase this value if the lighting condition is variable. **It is best not to change this value if you are not sure.**
   * *Light difference significance factor*: factor used by the stream analyzer to decide if the IR emitter is working. A larger value can lead to fewer false positives if the light condition is variable. **It is best not to change this value if you are not sure.**
   * *Rejection threshold per control*: after how many failed attempts the byte control under test is considered not the one that enables the IR emitter. A larger value can lead to more reliable results, but will take more time. **Increase this value a bit if the configuration is failing.**
   * *Increment step*: increment step used to explore the control space (at least 1). A larger value will make the search faster, but can miss some possibilities. **Increase this value a bit if the configuration is taking too long.**
5. *Continue* and read carefully the popup message.
   > [!WARNING]
   > The camera firmware may get corrupted during the process. You acknowledge that you understand the risks and that you are the only responsible if something goes wrong.
6. Answer the asked questions by pressing Y or N. There is a camera preview that can help to see if the IR emitter is blinking (the video will flash).
7. Sometimes, the tool can exit and request you to shut down, then boot and retry ($\neq$ reboot), in case a camera control was not able to be reset. Next time, that control will be skipped.
8. :crossed_fingers: Let's wait a bit... Hopefully, your IR emitter is now working! The control that enabled it is saved, you can see it with `linux-enable-ir-emitter --config`.

:star: If you like the project, do not hesitate to star the repository to support me, thank you!

> [!TIP]
> If the configuration failed, you can retry by tweaking some search parameters (read the section above) or use the advanced *tweak* tool (see below). Feel free to open an issue if you need any help! I will be glad to assist you. Please join the output of `linux-enable-ir-emitter --log` to it.

All criticism, ideas and contributions are welcome!

## How do I tweak my camera?

Some cameras provide UVC controls for other things than just turning the IR emitter on, like changing its brightness. Or maybe the *IR Enabler* did not manage to find the right value for your camera. In these cases, you can find the corresponding controls and set the correct value manually with the *UVC Tweaker* tool.

A control is identified by a *unit* and a *selector*, and consists of one or several bytes, each one being a value between 0-255.

0. For a better experience, use a large terminal window.
1. Execute the command: `linux-enable-ir-emitter configure`
2. Select the *UVC Tweaker* tool.
3. (Optional) Configure your *Device settings*, they are the same as for the *IR Enabler* (read the section above), except the *Number of emitters* which is not needed here.
4. *Continue*. The camera preview starts directly, but listing the UVC controls can take a few seconds (*Loading the UVC controls...*), the tool has to ask the camera for every possible control.
5. For each control, the list shows:
   * *initial*: the value when the tool started.
   * *current*: the value currently applied to the camera.
   * *maximum*: the maximum value of each byte, if the camera reports it.
6. Navigate between the controls with the arrow keys or Tab, and press Enter to *Edit* one.
7. While editing, use Left/Right to select a byte, and Up/Down to increase/decrease its value. **The value is directly applied to the camera**, so you can watch the camera preview (or the IR emitter itself) to see the effect. A byte cannot go above its *maximum*.
8. Press Enter to *Save* the value and go back to the list of controls.
9. If the camera refuses a value, an *Error* popup is shown and the control is reset to its previous value. Press any key to close it. Some controls are read-only, you can see which ones with `linux-enable-ir-emitter --grey-devices` (`writable=false`).
10. Press Esc when you are done, and answer whether you want to save the configuration by pressing Y or N:
    * Y: only the controls whose *current* value is different from their *initial* value are saved, they will be applied by `linux-enable-ir-emitter run`. If a control was already saved before (e.g. by the *IR Enabler*), its value is replaced.
    * N: nothing is saved. The modified values stay applied to the camera until it is reset (e.g. after a shutdown).
    * Any other key: go back to the list of controls.
11. You can check what has been saved with `linux-enable-ir-emitter --config`.

> [!WARNING]
> Same as for the *IR Enabler*: the camera firmware may get corrupted by writing random values to the controls. You acknowledge that you understand the risks and that you are the only responsible if something goes wrong.
