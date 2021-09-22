import sys

from IrConfiguration import load_saved_config
from command.configure import exitIfFileDescriptorError
from globals import ExitCode


def execute():
    """Apply the saved configuration"""
    ir_config = load_saved_config()
    exit_code = sys.exit(ExitCode.FAILURE) if not ir_config else ir_config.run()
    exitIfFileDescriptorError(exit_code, ir_config.device)
    sys.exit(exit_code)
    