#!/bin/sh

apply() {
    semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/bin/execute-driver
    semanage fcontext -a -t bin_t /usr/lib/linux-enable-ir-emitter/bin/driver-generator
    restorecon -v /usr/lib64/linux-enable-ir-emitter/bin/*
}

undo() {
    semanage fcontext -d /usr/lib/linux-enable-ir-emitter/driver/execute-driver
    semanage fcontext -d /usr/lib/linux-enable-ir-emitter/driver/driver-generator
}


if [ "$EUID" -ne 0 ]; then
    echo "Please run as root."
    exit 1
fi

which semanage &> /dev/null
if [ "$?" -ne 0 ]; then
    echo "SELinux is not installed on your system."
    exit 1
fi

case "$1" in
    "apply")
        apply
        ;;
    "undo")
        undo
        ;;
    *)
        echo "usage: shell fix_SELinux.sh {apply, undo}."
        exit 1
        ;;
esac
