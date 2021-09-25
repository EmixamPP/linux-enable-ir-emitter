import os
import logging
import subprocess
import sys

from IrConfiguration import IrConfiguration
from IrConfigurationSerializer import IrConfigurationSerializer
from globals import SAVE_CONFIG_FILE_PATH, EDITOR_PATH, ExitCode


def execute():
    """Display the current configuration in the default editor"""
    if not os.path.exists(SAVE_CONFIG_FILE_PATH):
        dummy_config = IrConfiguration([0], 0, 0, "/dev/videoX")
        IrConfigurationSerializer.save_all_config([dummy_config])

    try:
        subprocess.run([EDITOR_PATH, SAVE_CONFIG_FILE_PATH])
    except FileNotFoundError:
        logging.critical("No editor found, set the envion variable 'EDITOR' or install nano.")
        sys.exit(ExitCode.MISSING_DEPENDENCY)
    
    sys.exit(ExitCode.SUCCESS)
