import os
import enum


LOCAL_PATH = path = os.path.dirname(os.path.abspath(__file__))

def _getSaveConfigFilePath():
    old_version_path = LOCAL_PATH + "/irConfig.yaml"
    actual_version_path = "/etc/linux-enable-ir-emitter.yaml"
    if os.path.exists(old_version_path): 
        return old_version_path
    return actual_version_path
SAVE_CONFIG_FILE_PATH = _getSaveConfigFilePath()

UVC_DIR_PATH = LOCAL_PATH + "/uvc/"
UVC_LEN_QUERY_PATH = UVC_DIR_PATH + "len_query"
UVC_GET_QUERY_PATH = UVC_DIR_PATH + "get_query"
UVC_SET_QUERY_PATH = UVC_DIR_PATH + "set_query"

SYSTEMD_NAME = "linux-enable-ir-emitter.service"

EDITOR_PATH =  os.environ["EDITOR"] if "EDITOR" in os.environ else "/usr/bin/nano"


class ExitCode(enum.IntEnum):
    SUCCESS = 0
    FAILURE = 1
    FILE_DESCRIPTOR_ERROR = 126
    MISSING_DEPENDENCY = 3
    ROOT_REQUIRED = 2