#!/bin/bash

usage() {
    columnPrint="%-20s%-s\n"
    printf "This is a simple tool to install/uninstall the sofware.\n"
    printf "usage: bash installer {install, uninstall}\n"
}

install_dependency() {
    check_root
    umask 022  # not really clean but it's the easiest way to install dependencies for all users using pip
    pip install opencv-python pyyaml
}

do_install() {
    check_root
    install_dependency
    make -C sources/driver/uvc

    # software
    install -Dm 644 sources/*.py -t /usr/lib/linux-enable-ir-emitter/ -v
    install -Dm 644 sources/command/*.py -t /usr/lib/linux-enable-ir-emitter/command/ -v
    install -Dm 644 sources/driver/*.py -t /usr/lib/linux-enable-ir-emitter/driver/ -v

    install -Dm 755 sources/driver/uvc/*query  -t /usr/lib/linux-enable-ir-emitter/driver/uvc/ -v
    install -Dm 755 sources/driver/uvc/*query.o  -t /usr/lib/linux-enable-ir-emitter/driver/uvc/ -v
    
    # boot service
    [[ $(cat /etc/os-release | grep 'debian') ]] &&
        install -Dm 644 sources/linux-enable-ir-emitter.service -t /lib/systemd/system/ -v ||
        install -Dm 644 sources/linux-enable-ir-emitter.service -t /usr/lib/systemd/system/ -v

    # executable
    chmod +x /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py
    ln -fs /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py /usr/bin/linux-enable-ir-emitter
}

do_uninstall() {
    check_root

    rm -fv /usr/bin/linux-enable-ir-emitter 
    rm -rfv /usr/lib/linux-enable-ir-emitter/
    rm -fv /usr/lib/systemd/system/linux-enable-ir-emitter.service
    rm -fv /lib/systemd/system/linux-enable-ir-emitter.service
    rm -fv /etc/linux-enable-ir-emitter.yaml
}

check_root() {
    if [ "$EUID" -ne 0 ]; then
        echo "Please run as root."
        exit 1
    fi
}

case "$1" in
"uninstall")
    do_uninstall
    ;;
"install")
    do_install
    ;;
*)
    usage
    exit 1
    ;;
esac
