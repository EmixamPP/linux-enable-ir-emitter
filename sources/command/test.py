import logging
import sys

from globals import ExitCode
from IrConfiguration import load_saved_config


def execute():
    """Try to trigger the ir emitter with the current configuration"""
    ir_config = load_saved_config()
    exit_code = ExitCode.FAILURE
    if ir_config:
        exit_code = ir_config.triggerIr()
        
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        logging.critical("Cannot access to %s.", ir_config.device)
    sys.exit(exit_code)