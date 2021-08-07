#!/bin/bash

usage() {
    columnPrint="%-20s%-s\n"

    printf "This is a simple tool to install/uninstall the sofware and help repair some problems cased by previous config.\n\n"
    printf "usage: bash installer option\n\n"

    printf "option:\n"
    printf "$columnPrint" "  install" "install linux-enable-ir-emitter"
    printf "$columnPrint" "  optional" "install the optional Python dependencies"
    printf "$columnPrint" "  uninstall" "uninstall linux-enable-ir-emitter and optional Python dependencies"
    printf "$columnPrint" "  repair" "uninstall chicony-ir-toggle if possible and install linux-enable-ir-emitter"
}

install_dependency() {
    check_sudo
    umask 022  # not really clean but it's the easy way to install dependencies for all users using pip
    pip install opencv-python pyyaml
}

install_opt_dependency() {
    check_sudo
    umask 022  # not really clean but it's the easy way to install dependencies for all users using pip
    pip install pyshark
}

do_install() {
    check_sudo
    cd sources && make 

    install -Dm 755 enable-ir-emitter  -t /usr/lib/linux-enable-ir-emitter/
    install -Dm 644 config.yaml -t /usr/lib/linux-enable-ir-emitter/
    install -Dm 755 *.py -t /usr/lib/linux-enable-ir-emitter/
    install -Dm 644 linux-enable-ir-emitter.service -t /usr/lib/systemd/system/
    ln -fs /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py /usr/bin/linux-enable-ir-emitter

    install_dependency
}

do_uninstall() {
    check_sudo

    rm -f /usr/bin/linux-enable-ir-emitter
    rm -rf /usr/lib/linux-enable-ir-emitter/
    rm -f /usr/lib/systemd/system/linux-enable-ir-emitter.service

    umask 022  # not really clean but it's the easy way to install dependencies for all users using pip
    pip uninstall pyshark -y  # not safe for other software, but certainly only used by mine
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
"optional")
    install_opt_dependency
    ;;
*)
    usage
    exit 1
    ;;
esac
