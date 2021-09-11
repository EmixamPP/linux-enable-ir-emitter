import os
import enum


LOCAL_PATH = path = os.path.dirname(os.path.abspath(__file__))
SAVE_CONFIG_FILE_PATH = LOCAL_PATH + "/irConfig.yaml"
CONFIG_FILE_PATH = LOCAL_PATH + "/config.yaml"

BIN_PATH = LOCAL_PATH + "/enable-ir-emitter"

SYSTEMD_NAME = "linux-enable-ir-emitter.service"
SYSTEMD_FILE_PATH = "/usr/lib/systemd/system/" + SYSTEMD_NAME

EDITOR_PATH =  os.environ["EDITOR"] if "EDITOR" in os.environ else "/usr/bin/nano"


class ExitCode(enum.IntEnum):
    SUCCESS = 0
    FAILURE = 1
    FILE_DESCRIPTOR_ERROR = 2
    MISSING_DEPENDENCY = 3
    PERMISSION_DENIED = 126