#!/bin/bash

usage() {
    columnPrint="%-20s%-s\n"
    printf "This is a simple tool to install/uninstall/update the sofware.\n"
    printf "usage: bash installer {install, uninstall, update}\n"
}

do_install() {
    check_root
    make -C sources/driver/

    # software
    install -Dm 644 sources/*.py -t /usr/lib/linux-enable-ir-emitter/ -v
    install -Dm 644 sources/command/*.py -t /usr/lib/linux-enable-ir-emitter/command/ -v
    install -Dm 555 sources/driver/driver-generator -t /usr/lib/linux-enable-ir-emitter/driver/ -v
    install -Dm 555 sources/driver/execute-driver -t /usr/lib/linux-enable-ir-emitter/driver/ -v
    
    # executable
    chmod 755 /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py
    ln -fs /usr/lib/linux-enable-ir-emitter/linux-enable-ir-emitter.py /usr/bin/linux-enable-ir-emitter

    # auto complete for bash
    install -Dm 644 sources/autocomplete/linux-enable-ir-emitter -t /usr/share/bash-completion/completions/

    # drivers folder
    mkdir -p /etc/linux-enable-ir-emitter/

    do_post_install
}

do_post_install() {
    # support update v3 to v4 
    if [ -f /etc/linux-enable-ir-emitter.yaml ]; then 
        python /usr/lib/linux-enable-ir-emitter/migrate-v3.py
    fi

    # if SELinux is installed, fix denied access to /dev/video
    command -v chcon &> /dev/null
    if [ "$?" -eq 0 ]; then
        chcon -t bin_t /usr/lib/linux-enable-ir-emitter/driver/execute-driver /usr/lib/linux-enable-ir-emitter/driver/driver-generator
    fi
}

do_uninstall() {
    check_root

    rm -fv /usr/bin/linux-enable-ir-emitter 
    rm -fv /usr/share/bash-completion/completions/linux-enable-ir-emitter
    rm -rfv /usr/lib/linux-enable-ir-emitter/
    rm -rfv /etc/linux-enable-ir-emitter/
    rm -fv /etc/linux-enable-ir-emitter.yaml
    rm -fv /usr/lib/systemd/system/linux-enable-ir-emitter.service
    rm -fv /etc/udev/rules.d/99-linux-enable-ir-emitter.rules
}

do_update() {
    check_root

    mv /etc/linux-enable-ir-emitter/ /tmp/
    do_uninstall
    mv /tmp/linux-enable-ir-emitter/ /etc/

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
"update")
    do_update
    ;;
*)
    usage
    exit 1
    ;;
esac
