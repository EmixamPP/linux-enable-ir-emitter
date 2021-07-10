#!/bin/bash

usage() {
    columnPrint="%-20s%-s\n"

    printf "This is a simple tool to install/uninstall the sofware and help repair some problems cased by previous config.\n\n"
    printf "usage: bash installer option \n\n"

    printf "option:\n"
    printf "$columnPrint" "  install" "install linux-enable-ir-emitter"
    printf "$columnPrint" "  uninstall" "uninstall linux-enable-ir-emitter"
    printf "$columnPrint" "  repair" "uninstall chicony-ir-toggle if possible and reinstall linux-enable-ir-emitter"
}

install_dependency() {
    git clone https://github.com/KimiNewt/pyshark.git
    cd pyshark/src
    python3 setup.py install
}

do_install() {
    check_sudo
    cd sources && make

    install -Dm 755 -t /usr/lib/linux-enable-ir-emiter/ enable-ir-emitter
    install -Dm 644 -t /usr/lib/linux-enable-ir-emiter/ config.yaml
    install -Dm 755 -t /usr/lib/linux-enable-ir-emiter/ *.py
    install -Dm 644 -t /usr/lib/systemd/system/ linux-enable-ir-emitter.service
    ln -fs /usr/lib/linux-enable-ir-emiter/linux-enable-ir-emitter.py /usr/bin/linux-enable-ir-emitter

    install_dependency
}

do_uninstall() {
    check_sudo

    rm -f /usr/bin/linux-enable-ir-emiter
    rm -rf /usr/lib/linux-enable-ir-emiter/
    rm -f /usr/lib/systemd/system/linux-enable-ir-emitter.service
}

do_repair() {
    check_sudo

    rm -f /usr/local/bin/chicony-ir-toggle
    rm -f /lib/udev/rules.d/99-ir-led.rules
    rm -f /lib/systemd/system-sleep/ir-led.sh

    do_install
}

check_sudo() {
    if [ "$EUID" -ne 0 ]; then
        echo "Please run as sudo"
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
"repair")
    do_repair
    ;;
*)
    usage
    exit 1
    ;;
esac
