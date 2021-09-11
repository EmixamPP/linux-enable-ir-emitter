import sys
import os
import yaml

from IrConfiguration import IrConfiguration
from command.quick import _show_config_test
from globals import ExitCode, CONFIG_FILE_PATH


def _show_contribution(ir_config):
    """Check if the configuration does not exist is not in the configuration file. 
    Then the user is invited to share it on Github.

    Args:
        ir_config (IrConfiguration): the configuration to be compared with those in the YAML config file
    """
    with open(CONFIG_FILE_PATH, "r") as config_file:
        config_list = yaml.load(config_file, Loader=yaml.Loader)

    for config in config_list:
        ir_config_to_compare = IrConfiguration(config["data"], config["unit"], config["selector"], ir_config.videoPath)
        if ir_config_to_compare == ir_config: # if it's not a new config
            return

    print("You can contribute to the project !")
    print("Take 5 min to paste the contents of 'linux-enable-ir-emitter manual' into a new issue: https://github.com/EmixamPP/linux-enable-ir-emitter/issues.")
    print("Thank you for the others !")


def execute(video_path, file_path=None):
    """Try to find the ir emitter configuration by capturing the bus, 
    on condition that the ir emitter is triggered via a Windows VM.
    
    The capture can also be done by the user, a .cap file must be given.
    (no bus sniffing -> Windows VM not required)

    Exit if pysharsk is not installed.

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
        file_path (string): Path to the .cap file for analysis. If None, use the sniff mode.

    Returns:
        ExitCode: ExitCode.FAILURE, ExitCode.SUCCESS, ExitCode.FILE_DESCRIPTOR_ERROR, ExitCode.MISSING_DEPENDENCY
    """
    try:
        from IrConfigCapture import IrConfigSniffer, IrConfigReader
    except ImportError:
        print("The 'pyshark' python dependency is required for this command.", file=sys.stderr)
        print("Please consult https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration.")
        return ExitCode.MISSING_DEPENDENCY

    if not file_path:  # sniffing mode
        input("Please read and folow this tutorial: https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration. Press enter when you are ready ")
        capture = IrConfigSniffer(video_path)
        capture.sniff(45)
        input("The capturing is finished, make sure the camera is connected to the host os. Press enter when it's done ")
    else:  # file reading mode
        if not os.path.isfile(file_path):
            print("The file {} doesn't exists.".format(file_path), file=sys.stderr)
            return ExitCode.FAILURE
        capture = IrConfigReader(video_path, file_path)           

    for ir_config in capture.config_list:
        exit_code = _show_config_test(ir_config)
        if exit_code == ExitCode.SUCCESS:
            _show_contribution(ir_config)
            return exit_code
        elif exit_code == ExitCode.FILE_DESCRIPTOR_ERROR:
            return exit_code

    print("No configuration was found.", file=sys.stderr)
    return ExitCode.FAILURE