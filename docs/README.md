### In this documentations you can find help for:
* [Solve common problem and instruction for opening a new issue](docs/issues.md)
* [System, manual build and hardware requirements](docs/requirements.md)
* [Exit code meaning](docs/exit-code.md)
* [Command line usage](#linux-enable-ir-emitter-usage) 

### linux-enable-ir-emitter usage 
```
usage: linux-enable-ir-emitter [-h] [-v] [-V] [-d device] {run,configure,delete,boot} ...

Provides support for infrared cameras.

positional arguments:
  {run,configure,delete,boot}
    run                 apply drivers
    configure           generate ir emitter's driver
    delete              delete drivers
    boot                enable ir at boot

options:
  -h, --help            show this help message and exit
  -v, --verbose         print verbose information
  -V, --version         show version information and exit
  -d device, --device device
                        specify the infrared camera, by default is '/dev/video2'
```
```
usage: linux-enable-ir-emitter configure [-h] [-e <count>] [-l <count>]

options:
  -h, --help            show this help message and exit
  -e <count>, --emitters <count>
                        the number of emitters on the device, by default is 1
  -l <count>, --limit <count>
                        the number of negative answer before the pattern is skiped, by default is 5. Use 256 for unlimited
```
```
usage: linux-enable-ir-emitter boot [-h] {enable,disable,status}

positional arguments:
  {enable,disable,status}
                        specify the boot action to perform

optional arguments:
  -h, --help            show this help message and exit
```