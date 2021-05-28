#!/bin/bash

BASE_PATH=$(realpath $0)
BASE_PATH="${BASE_PATH%install.bash}"
PATH_CMD="${HOME}/.local/bin/"

SERVICE_ENABLE_IR_PATH="${BASE_PATH}enable-ir-emitter.service"

SCRIPT_PATH="${BASE_PATH}linux-enable-ir-emitter"

gcc ${BASE_PATH}enable-ir-emitter.c -o ${BASE_PATH}enable-ir-emitter
ln -s ${BASE_PATH}linux-enable-ir-emitter $PATH_CMD

printf "[Unit]\nDescription=enable ir emitter\nAfter=multi-user.target suspend.target hibernate.target hybrid-sleep.target suspend-then-hibernate.target\n\n[Service]\nExecStart=$SCRIPT_PATH -f\n\n[Install]\nWantedBy=multi-user.target suspend.target hibernate.target hybrid-sleep.target suspend-then-hibernate.target" >$SERVICE_ENABLE_IR_PATH
