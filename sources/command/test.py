import logging
import sys

from IrConfigurationSerializer import IrConfigurationSerializer
from command.configure import exitIfFileDescriptorError
from globals import ExitCode


def execute():
    """Try to trigger the ir emitter with the current configuration"""
    ir_config_list = IrConfigurationSerializer.load_all_saved_config()
    if not ir_config_list : sys.exit(ExitCode.FAILURE)

    for ir_config in ir_config_list:
        exit_code = ir_config.triggerIr()
        exitIfFileDescriptorError(exit_code, ir_config.device)
        if exit_code != ExitCode.SUCCESS:
            logging.critical("Bad configuration for %s.", ir_config.device)
            sys.exit(exit_code)
        
    sys.exit(ExitCode.SUCCESS)