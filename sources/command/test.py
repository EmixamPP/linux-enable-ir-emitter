from globals import ExitCode
from IrConfiguration import load_saved_config

def execute():
    """Try to trigger the ir emitter with the current configuration
    
    Returns: 
        ExitCode: ExitCode.SUCCESS, ExitCode.FAILURE or ExitCode.FILE_DESCRIPTOR_ERROR
    """
    ir_config = load_saved_config()
    exit_code = ExitCode.FAILURE
    if ir_config:
       exit_code = ir_config.trigger_ir(2)
    return exit_code