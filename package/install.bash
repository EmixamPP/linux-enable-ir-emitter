#!/bin/bash

BASE_PATH=$(realpath $0)
BASE_PATH="${BASE_PATH%install.bash}"
PATH_CMD="${HOME}/.local/bin/"

gcc ${BASE_PATH}enable-ir-emitter.c -o ${BASE_PATH}enable-ir-emitter
ln -s ${BASE_PATH}linux-enable-ir-emitter $PATH_CMD
