### In this documentations you can find help for:
* [Solve common problem and instruction for opening a new issue](issues.md)
* [Save, reset and delete your camera configurations](configurations.md)
* [System, manual build and hardware requirements](requirements.md)
* [Manual build instructions](manual-build.md)
* [Uninstallation](uninstallation.md)
* [Exit code meaning](exit-code.md)
* [Command line usage](#linux-enable-ir-emitter-usage)

### linux-enable-ir-emitter usage
```
Usage: linux-enable-ir-emitter [--help] [--version VAR] [--device device] [--width width] [--height height] [--verbose] {configure,run,test,tweak}

Provides support for infrared cameras.

Optional arguments:
  -h, --help           shows help message and exits
  -V, --version        shows version message and exits
  -d, --device device  specify the camera, nothing for automatic [default: ""]
  -w, --width width    specify the width, -1 automatic [default: -1]
  -t, --height height  specify the height, -1 for automatic [default: -1]
  -v, --verbose        enables verbose information

Subcommands:
  configure
  run
  test
  tweak

```
```
Usage: configure [--help] [--version] [--manual] [--emitters emitters] [--limit limit] [--no-gui]

Optional arguments:
  -h, --help               shows help message and exits
  -v, --version            prints version information and exits
  -m, --manual             manual verification
  -e, --emitters emitters  specify the number of emitters [default: 1]
  -l, --limit limit        specify the negative answer limit, use -1 for unlimited [default: 10]
  -g, --no-gui             disable video feedback
```
