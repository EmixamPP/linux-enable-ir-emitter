import sys
import logging

from IrConfiguration import load_saved_config
from globals import ExitCode


def execute():
    """Run the config saved in irConfig.yaml"""
    ir_config = load_saved_config()
    exit_code = ExitCode.FAILURE
    if ir_config:
        exit_code = ir_config.run()
        
    if exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
        logging.critical("Cannot access to %s.", ir_config.device)
    sys.exit(exit_code)
    