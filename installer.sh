#!/bin/bash

usage() {
    columnPrint="%-20s%-s\n"
    printf "This is a simple tool to install/reinstall/uninstall/update the sofware.\n"
    printf "usage: bash installer {install, reinstall, uninstall, update}\n"
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
    
    # executable
    chmod +x /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py
    ln -fs /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py /usr/bin/linux-enable-ir-emitter
}

do_uninstall() {
    check_root

    rm -fv /usr/bin/linux-enable-ir-emitter 
    rm -rfv /usr/lib/linux-enable-ir-emitter/
    rm -fv /usr/lib/systemd/system/linux-enable-ir-emitter.service
    rm -fv /etc/udev/rules.d/99-linux-enable-ir-emitter.rules
    rm -fv /etc/linux-enable-ir-emitter.yaml
}

do_reinstall() {
    check_root

    mv /etc/linux-enable-ir-emitter.yaml /tmp/
    do_uninstall
    mv /tmp/linux-enable-ir-emitter.yaml /etc/

    do_install
}

do_update() {
    check_root

    mv /etc/linux-enable-ir-emitter.yaml /tmp/
    do_uninstall
    mv /tmp/linux-enable-ir-emitter.yaml /etc/

    git fetch && git pull
    do_install
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
"reinstall")
    do_reinstall
    ;;
"update")
    do_update
    ;;
*)
    usage
    exit 1
    ;;
esac
