import yaml
import os
import sys

from sources.IrConfiguration import IrConfiguration
from sources.IrConfigCapture import IrConfigCapture

local_path = path = os.path.dirname(os.path.abspath(__file__))
config_file_path = local_path + "/config.yaml"
save_config_file_path = local_path + "/irConfig.yaml"

systemd_name = "linux-enable-ir-emitter.service"
systemd_file_path = local_path + systemd_name


def _show_config_test(ir_config):
    """Test the configuration and ask the user if it works. In this case the script is exited.

    Args:
        ir_config (IrConfiguration): configuration to test
    """
    if not ir_config.run():
        ir_config.trigger_ir()
        check = input(
            "Did you see the ir emitter flashing ? Yes/No ? ").lower()

        if check in ("yes", "y"):
            print("A configuration have been found. Here is what you can do:")
            print(
                "\t- activate the emitter at system startup : 'linux-ir-emitter boot enable'")
            print(
                "\t- manually activate the emitter for one session : 'linux-ir-emitter run'")
            ir_config.save(local_path)
            sys.exit(0)


def quick(video_path):
    """Use the shared configuration file to set up the ir emitter

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    with open(config_file_path, "r") as config_file:
        config_list = yaml.load(config_file, Loader=yaml.Loader)

    i = 0
    while i < len(config_list):
        config = config_list[i]
        ir_config = IrConfiguration(
            config["data"], config["unit"], config["selector"], video_path)

        _show_config_test(ir_config)
        i += 1

    print("No configuration was found please execute : 'linux-ir-emitter full'", file=sys.stderr)


def run():
    """Run the config saved in irConfig.yaml
    """
    try:
        if os.path.exists(save_config_file_path):
            ir_config = IrConfiguration.load(save_config_file_path)
            ir_config.run()
        else:
            print("No configuration is currently saved", file=sys.stderr)
    except yaml.YAMLError:
        print("The config file is corrupted !", file=sys.stderr)


def boot(status):
    """Enable or disable the systemd service which activates the ir emitter

    Args:
        status (string): "enable" or "disable"
    """
    if status == "enable":
        os.system("systemctl enable --now {}".format(systemd_file_path))
    elif status == "disable":
        os.system("systemctl disable --now {}".format(systemd_name))
    else:
        raise Exception("status arg can only be equal to enable or disable")


def manual(video_path):
    """Display the current configuration in the nano editor

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    if not os.path.exists(save_config_file_path):
        IrConfiguration([0], 0, 0, video_path).save(local_path)
    os.system("/bin/nano " + save_config_file_path)


def test():
    """Try to trigger the ir emitter with the current configuration
    """
    try:
        if os.path.exists(save_config_file_path):
            ir_config = IrConfiguration.load(save_config_file_path)
            ir_config.run()
            ir_config.trigger_ir(2)
        else:
            print("No configuration is currently saved", file=sys.stderr)
    except yaml.YAMLError:
        print("The config file is corrupted !", file=sys.stderr)


def full(video_path):
    """Try to find the ir emitter configuration by capturing the bus.
    On condition that the ir emitter is triggered via a Windows VM.

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    input("Please read and folow this tutorial : https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration. Press enter when you'r ready")
    capture = IrConfigCapture(video_path)
    capture.start(60)

    input("The capturing is finished, make sure the camera is connected to the host os. Press enter when it's done ")
    for ir_config in capture.config_list:
        _show_config_test(ir_config)

    print("No configuration was found", file=sys.stderr)
    sys.exit(1)
