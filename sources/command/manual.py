import os
import sys
import subprocess

from IrConfiguration import IrConfiguration, load_saved_config
from globals import ExitCode, SAVE_CONFIG_FILE_PATH, EDITOR_PATH


def execute(video_path):
    """Display the current configuration in the default editor

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"

    Returns: 
        ExitCode: ExitCode.SUCESS or ExitCode.MISSING_DEPENDENCY
    """
    dummy_config = IrConfiguration([0], 0, 0, video_path)
    if not os.path.exists(SAVE_CONFIG_FILE_PATH):
        dummy_config.save(SAVE_CONFIG_FILE_PATH)

    try:
        subprocess.call([EDITOR_PATH, SAVE_CONFIG_FILE_PATH])
    except FileNotFoundError:
        print("No editor found, set the envion variable 'EDITOR' or install nano.", file=sys.stderr)
        return ExitCode.MISSING_DEPENDENCY

    if load_saved_config() == dummy_config:
        os.system("rm " + SAVE_CONFIG_FILE_PATH)
    return ExitCode.SUCCESS