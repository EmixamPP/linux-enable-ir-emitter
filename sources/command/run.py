from IrConfiguration import load_saved_config
from globals import ExitCode

def execute():
    """Run the config saved in irConfig.yaml
    
    Returns: 
        ExitCode: ExitCode.SUCCESS, ExitCode.FAILURE or ExitCode.FILE_DESCRIPTOR_ERROR
    """
    ir_config = load_saved_config()
    exit_code = ExitCode.FAILURE
    if ir_config:
        exit_code = ir_config.run()
    return exit_code