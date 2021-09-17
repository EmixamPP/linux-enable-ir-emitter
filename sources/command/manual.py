import os
import logging
import subprocess
import sys

from IrConfiguration import IrConfiguration, load_saved_config
from globals import SAVE_CONFIG_FILE_PATH, EDITOR_PATH, ExitCode


def execute(device):
    """Display the current configuration in the default editor

    Args:
        device (str): Path to the infrared camera e.g : "/dev/video2"
    """
    dummy_config = IrConfiguration([0], 0, 0, device)
    if not os.path.exists(SAVE_CONFIG_FILE_PATH):
        dummy_config.save(SAVE_CONFIG_FILE_PATH)

    try:
        subprocess.run([EDITOR_PATH, SAVE_CONFIG_FILE_PATH])
    except FileNotFoundError:
        logging.critical("No editor found, set the envion variable 'EDITOR' or install nano.")
        sys.exit(ExitCode.MISSING_DEPENDENCY)

    if load_saved_config() == dummy_config:
        subprocess.run("rm " + SAVE_CONFIG_FILE_PATH, shell=True, capture_output=True)
    
    sys.exit(ExitCode.SUCCESS)
