#!/bin/bash

usage() {
    columnPrint="%-20s%-s\n"
    printf "Simple tool to install/uninstall linux-enable-ir-emitter.\n"
    printf "usage: bash installer.sh {install, uninstall}\n"
}

do_install() {
    check_root
    make -C sources/driver/

    # software
    install -Dm 644 sources/*.py -t /usr/lib64/linux-enable-ir-emitter/ -v
    install -Dm 644 sources/command/*.py -t /usr/lib64/linux-enable-ir-emitter/command/ -v
    install -Dm 755 sources/driver/driver-generator -t /usr/lib64/linux-enable-ir-emitter/driver/ -v
    install -Dm 755 sources/driver/execute-driver -t /usr/lib64/linux-enable-ir-emitter/driver/ -v
    
    # executable
    chmod 755 /usr/lib64/linux-enable-ir-emitter/linux-enable-ir-emitter.py
    ln -fs /usr/lib64/linux-enable-ir-emitter/linux-enable-ir-emitter.py /usr/bin/linux-enable-ir-emitter

    # auto complete for bash
    install -Dm 644 sources/autocomplete/linux-enable-ir-emitter -t /usr/share/bash-completion/completions/

    # drivers folder
    mkdir -p /etc/linux-enable-ir-emitter/

    do_post_install
}

do_post_install() {
    # if SELinux is installed, fix denied access to camera
    which semanage &> /dev/null
    if [ "$?" -eq 0 ]; then
        semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/driver/execute-driver
        semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/driver/driver-generator
        restorecon -v /usr/lib64/linux-enable-ir-emitter/driver/* 1> /dev/null
    fi
}

do_uninstall() {
    check_root
    
    which semanage &> /dev/null
    if [ "$?" -eq 0 ]; then
        semanage fcontext -d /usr/lib/linux-enable-ir-emitter/driver/execute-driver
        semanage fcontext -d /usr/lib/linux-enable-ir-emitter/driver/driver-generator
    fi

    rm -fv /usr/bin/linux-enable-ir-emitter 
    rm -fv /usr/share/bash-completion/completions/linux-enable-ir-emitter
    rm -rfv /usr/lib64/linux-enable-ir-emitter/
    rm -rfv /etc/linux-enable-ir-emitter/

    systemctl disable linux-enable-ir-emitter &> /dev/null
    rm -fv /usr/lib64/systemd/system/linux-enable-ir-emitter.service
    rm -fv /etc/udev/rules.d/99-linux-enable-ir-emitter.rules
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
