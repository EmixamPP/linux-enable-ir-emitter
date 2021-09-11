import sys
import yaml

from IrConfiguration import IrConfiguration
from globals import ExitCode, SAVE_CONFIG_FILE_PATH, CONFIG_FILE_PATH


def _show_config_test(ir_config, skip_quesiton=False):
    """Test the configuration and ask the user if it works. 
    In this case the configuration is saved and a help message is displayed with what the user can do. 

    Args:
        ir_config (IrConfiguration): configuration to test
        skip_quesiton (bool): if true skip all validations. Default to False.
    
    Returns:
        ExitCode: ExitCode.SUCCESS if the configuration works, else ExitCode.FAILURE or ExitCode.FILE_DESCRIPTOR_ERROR
    """
    exit_code = ir_config.trigger_ir()
    if exit_code == ExitCode.SUCCESS:
        check = "yes"
        if not skip_quesiton:
            check = input("Did you see the ir emitter flashing ? Yes/No ? ").lower()
            while (check not in ("yes", "y", "no", "n")):
                check = input("Yes/No ? ").lower()

        if check in ("yes", "y"):
            ir_config.save(SAVE_CONFIG_FILE_PATH)
            print("A configuration have been found. Here is what you can do:")
            print("  - activate the emitter at system boot : 'linux-enable-ir-emitter boot enable'")
            print("  - manually activate the emitter for one session : 'linux-enable-ir-emitter run'")
        else:
            exit_code = ExitCode.FAILURE

    return exit_code


def execute(video_path, skip_question):
    """Use the shared configuration file to set up the ir emitter

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
        skip_quesiton (bool): if true skip all validations

    Returns: 
        ExitCode: ExitCode.SUCCESS, ExitCode.FAILURE or ExitCode.FILE_DESCRIPTOR_ERROR
    """
    with open(CONFIG_FILE_PATH, "r") as config_file:
        config_list = yaml.load(config_file, Loader=yaml.Loader)

    for config in config_list:
        ir_config = IrConfiguration(config["data"], config["unit"], config["selector"], video_path)
        exit_code = _show_config_test(ir_config, skip_question)
        if exit_code in (ExitCode.SUCCESS, ExitCode.FILE_DESCRIPTOR_ERROR):
            return exit_code

    print("No configuration was found please execute : 'linux-enable-ir-emitter full'.", file=sys.stderr)
    return ExitCode.FAILURE