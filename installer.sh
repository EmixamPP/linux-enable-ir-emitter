#!/bin/bash

usage() {
    columnPrint="%-20s%-s\n"
    printf "This is a simple tool to install/uninstall the sofware.\n"
    printf "usage: bash installer {install, uninstall}\n"
}

install_dependency() {
    check_root
    umask 022  # not really clean but it's the easy way to install dependencies for all users using pip
    pip install opencv-python pyyaml
}

do_install() {
    check_root
    make -C sources/uvc

    # software
    install -Dm 755 sources/uvc/*query  -t /usr/lib/linux-enable-ir-emitter/uvc/
    install -Dm 755 sources/uvc/*query.o  -t /usr/lib/linux-enable-ir-emitter/uvc/

    install -Dm 644 sources/command/*.py -t /usr/lib/linux-enable-ir-emitter/command/

    install -Dm 644 sources/*.py -t /usr/lib/linux-enable-ir-emitter/

    # boot service
    install -Dm 644 sources/linux-enable-ir-emitter.service -t /usr/lib/systemd/system/

    # executable
    chmod +x /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py
    ln -fs /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py /usr/bin/linux-enable-ir-emitter

    install_dependency
}

do_uninstall() {
    check_root

    rm -f /usr/bin/linux-enable-ir-emitter
    rm -rf /usr/lib/linux-enable-ir-emitter/
    rm -f /usr/lib/systemd/system/linux-enable-ir-emitter.service
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
