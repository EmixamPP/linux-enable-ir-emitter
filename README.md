# <p align=center>linux-enable-ir-emitter</p>

Provides support for infrared cameras that are not directly enabled out-of-the box on Linux (at the very least, the kernel must recognize your infrared camera). The purpose of this repository is to enable the emitter when the infrared camera is invoked.

`linux-enable-ir-emitter` can automatically configure almost any UVC infrared camera.

> [!IMPORTANT]
> Please read the documentation below carefully. It can save you a lot of time and help you successfully enable your infrared camera.

> [!NOTE]
> If you plan to package this software for a Linux distribution, or to contribute code, please read [CONTRIBUTING.md](CONTRIBUTING.md).

## Installation
Download the latest [linux-enable-ir-emitter-x.x.x-release-x86-64.tar.gz](https://github.com/EmixamPP/linux-enable-ir-emitter/releases). Then execute:
> [!NOTE]
> Please try the 7.0.0-beta! Furthermore, this README has been updated for this version.

```
tar -C $HOME/.local/bin --no-same-owner -m -vxzf linux-enable-ir-emitter*.tar.gz
```
If not already done, add `$HOME/.local/bin` to your PATH, e.g.:
```
echo 'export PATH=$HOME/.local/bin:$PATH' >> $HOME/.bashrc && source $HOME/.bashrc
```

The installation consists of 3 files:
* The executable: `$HOME/.local/bin/linux-enable-ir-emitter`
* The logs (for receiving help): `$HOME/.local/state/linux-enable-ir-emitter.log`
* The camera configuration (can be backed up): `$HOME/.config/linux-enable-ir-emitter.toml`

### Integration with Howdy
In all files returned by `grep -rl howdy /etc/pam.d`, add the following line before the one mentioning "howdy", replacing `<USER>` with your actual username:
```
auth optional pam_exec.so /home/<USER>/.local/bin/linux-enable-ir-emitter run
```

The path to the binary may vary depending on your installation method. You can determine the correct absolute path by running `which linux-enable-ir-emitter` and use that path instead.

### Integration with other program
You will need to execute the `linux-enable-ir-emitter run` command before the program that uses the infrared camera.

Alternatively, if you can and/or want to integrate better with the program that uses the camera, you can pass an opened file descriptor for the camera to the command: `linux-enable-ir-emitter run --device <DEVICE> --fd <FD>`.

## How do I enable my infrared emitter?
1. `linux-enable-ir-emitter configure`
2. Select the *IR Enabler* tool. It will iterate through the UVC camera controls and modify them to try to find the one that enables the IR emitter.
3. (Optional) Configure your *Device settings*:
   * *Path*: by default, a grey camera will be selected (if any). If you have multiple cameras, you can change the path.
   * *Number of emitters*: if your camera has multiple IR emitters, specify it here.
   * *Emitters*: number of emitters on your camera.
   * *Resolution height/width*: by default, the camera driver will select a resolution for you. You can change it if needed.
   * *FPS*: by default, the camera driver will select a frame rate for you. You can change it if needed.
4. (Optional) Configure the *Search settings*:
   * *Manual mode*: if enabled, our stream analyzer will not be used to detect if the IR emitter is working. Instead, you will have to confirm manually using the camera preview or the IR emitter itself. **Enable this mode if the configuration failed the first time.**
   * *Images to analyze in auto validation*: number of images used by the stream analyzer to decide if the IR emitter is working. A larger value can lead to more reliable results, but will take more time. Increase this value if the lighting condition is variable. **It is best not to change this value if you are not sure.**
   * *Light difference significance factor*: factor used by the stream analyzer to decide if the IR emitter is working. A larger value can lead to fewer false positives if the light condition is variable. **It is best not to change this value if you are not sure.**
   * *Rejection threshold*: after how many failed attempts the byte control under test is considered not the one that enables the IR emitter. A larger value can lead to more reliable results, but will take more time. **Increase this value a bit if the configuration is failing.**
   * *Increment step*: increment step used to explore the control space. A larger value will make the search faster, but can miss some possibilities. **Increase this value a bit if the configuration is taking too long.**
5. *Continue* and read carefully the popup message.
   > [!WARNING]
   > The camera firmware may get corrupted during the process. You acknowledge that you understand the risks and that you are the only responsible if something goes wrong.
6. Answer the asked questions by pressing Y or N. There is a camera preview that can help to see if the IR emitter is blinking (the video will flash).
7. Sometimes, the tool can exit and request you to shut down, then boot and retry ($\neq$ reboot), in case a camera control was not able to be reset. Next time, that control will be skipped.
8. :crossed_fingers: Let's wait a bit... Hopefully, your IR emitter is now working!

:star: If you like the project, do not hesitate to star the repository to support me, thank you!

> [!TIP]
> If the configuration failed, you can retry by tweaking some search parameters (read the section above) or use the advanced *tweak* tool (see below). Feel free to open an issue if you need any help! I will be glad to assist you.


All criticism, ideas and contributions are welcome!

## How do I tweak my camera?
Note: Some cameras provide UVC controls for changing the brightness of the ir emitter.
You will need to find the corresponding controls and set the correct value manually.
A control consists of X values between 0-255.

**Under total rework, will come back soon**
