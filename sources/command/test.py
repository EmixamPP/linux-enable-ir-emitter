import sys

from globals import ExitCode
from command.configure import exitIfFileDescriptorError
from IrConfiguration import load_saved_config


def execute():
    """Try to trigger the ir emitter with the current configuration"""
    ir_config = load_saved_config()
    exit_code = sys.exit(ExitCode.FAILURE) if not ir_config else ir_config.triggerIr()
    exitIfFileDescriptorError(exit_code, ir_config.device)
    sys.exit(exit_code)