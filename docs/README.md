### In this documentations you can find help for:
* [Solve common problem and instruction for opening a new issue](issues.md)
* [System, manual build and hardware requirements](requirements.md)
* [Exit code meaning](exit-code.md)
* [Command line usage](#linux-enable-ir-emitter-usage) 

### linux-enable-ir-emitter usage 
```
usage: linux-enable-ir-emitter [-h] [-v] [-V] [-d device] [-w width] [-t height] {run,configure,tweak,test,boot,delete} ...

Provides support for infrared cameras.

positional arguments:
  {run,configure,tweak,test,boot,delete}
    run                 apply configurations
    configure           generate ir emitter configuration
    tweak               tweak a camera
    test                test a camera
    boot                enable ir at boot
    delete              delete configurations

options:
  -h, --help            show this help message and exit
  -v, --verbose         print verbose information
  -V, --version         show version information and exit
  -d device, --device device
                        specify the infrared camera, automatic detection by default
  -w width, --width width
                        specify the width of the camera, automatic by default
  -t height, --height height
                        specify the height of the camera, automatic by default
```
```
usage: linux-enable-ir-emitter configure [-h] [-m] [-e <count>] [-l <count>]

options:
  -h, --help            show this help message and exit
  -m, --manual          activate manual configuration
  -e <count>, --emitters <count>
                        the number of emitters on the device, by default is 1
  -l <count>, --limit <count>
                        the number of negative answer before the pattern is skipped, by default is 40. Use -1 for unlimited
  -g, --no-gui          no gui video feedback
```
```
usage: linux-enable-ir-emitter boot [-h] {enable,disable,status}

positional arguments:
  {enable,disable,status}
                        specify the boot action to perform

optional arguments:
  -h, --help            show this help message and exit
```
