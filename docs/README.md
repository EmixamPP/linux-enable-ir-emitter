### In this documentations you can find help for:
* [Solve common problem and instruction for opening a new issue](issues.md)
* [System, manual build and hardware requirements](requirements.md)
* [Manual build instructions](manual-build.md)
* [Exit code meaning](exit-code.md)
* [Command line usage](#linux-enable-ir-emitter-usage) 

### linux-enable-ir-emitter usage 
```
usage: linux-enable-ir-emitter [-h] [-V] [-v] [-d device] [-w width] [-t height] {run,configure,tweak,test,boot} ...

Provides support for infrared cameras.

positional arguments:
  {run,configure,tweak,test,boot}
    run                 apply a configuration
    configure           create an ir emitter configuration
    tweak               create a camera configuration
    test                test a camera
    boot                apply the configurations at boot

options:
  -h, --help            show this help message and exit
  -V, --version         show version information and exit
  -v, --verbose         print verbose information
  -d device, --device device
                        specify the camera, automatic by default
  -w width, --width width
                        specify the width, automatic by default
  -t height, --height height
                        specify the height, automatic by default
```
```
usage: linux-enable-ir-emitter configure [-h] [-m] [-e <count>] [-l <count>] [-g]

options:
  -h, --help            show this help message and exit
  -m, --manual          manual verification
  -e <count>, --emitters <count>
                        specify the number of emitters, by default is 1
  -l <count>, --limit <count>
                        specify the negative answer limit, by default is 40. Use -1 for unlimited
  -g, --no-gui          disable video feedback
```
```
usage: linux-enable-ir-emitter boot [-h] {enable,disable,status}

positional arguments:
  {enable,disable,status}
                        specify the boot action to perform

optional arguments:
  -h, --help            show this help message and exit
```
