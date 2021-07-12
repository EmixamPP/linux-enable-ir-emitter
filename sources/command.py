import yaml
import os
import sys

from IrConfiguration import IrConfiguration
from IrConfigCapture import IrConfigCapture

local_path = path = os.path.dirname(os.path.abspath(__file__))
config_file_path = local_path + "/config.yaml"
save_config_file_path = os.path.expanduser("~") + "/.irConfig.yaml"

systemd_name = "linux-enable-ir-emitter.service"
systemd_file_path = "/usr/lib/systemd/system/" + systemd_name


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
        status (string): "enable" or "disable" or "status"

    Raises:
        Exception: status arg can only be equal to enable or disable
    """
    if os.path.exists(systemd_file_path):
        if status in ("enable", "disable", "status"):
            os.system("systemctl {} --now {}".format(status, systemd_name))
        else:
            raise Exception("status arg can only be equal to enable or disable")
    else:
        print("Please install linux-enable-ir-emitter first", file=sys.stderr)


def manual(video_path):
    """Display the current configuration in the nano editor

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    dummy_config = IrConfiguration([0], 0, 0, video_path)
    if not os.path.exists(save_config_file_path):
        dummy_config.save(local_path)

    os.system("/bin/nano " + save_config_file_path)
    actual_config = IrConfiguration.load(save_config_file_path)
    if actual_config == dummy_config:
        os.system("rm " + save_config_file_path)


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


def _show_config_test(ir_config, save=True):
    """Test the configuration and ask the user if it works. In this case the coinfiguration is saved.

    Args:
        ir_config (IrConfiguration): configuration to test
        save (bool, optional): save the config if it works. Defaults to True.

    Returns:
        bool: True if the configuration works, else False
    """
    if not ir_config.run():
        ir_config.trigger_ir()
        check = input(
            "Did you see the ir emitter flashing ? Yes/No ? ").lower()

        if check in ("yes", "y"):
            if save:
                ir_config.save(save_config_file_path)
            return True
    return False


def quick(video_path):
    """Use the shared configuration file to set up the ir emitter

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    with open(config_file_path, "r") as config_file:
        config_list = yaml.load(config_file, Loader=yaml.Loader)

    for config in config_list:
        ir_config = IrConfiguration(
            config["data"], config["unit"], config["selector"], video_path)

        if _show_config_test(ir_config):
            print("A configuration have been found. Here is what you can do:")
            print("\t- activate the emitter at system startup : 'linux-ir-emitter boot enable'")
            print("\t- manually activate the emitter for one session : 'linux-ir-emitter run'")
            return

    print("No configuration was found please execute : 'linux-ir-emitter full'", file=sys.stderr)


def _is_new_config(ir_config):
    """Check if the configuration does not exist is not in the configuration file. Then the user is invited to share it on Github

    Args:
        ir_config (IrConfiguration): the configuration to be compared with those in the YAML config file

    Returns:
        bool: True if the ir_config is new, else False
    """
    with open(config_file_path, "r") as config_file:
        config_list = yaml.load(config_file, Loader=yaml.Loader)

    for config in config_list:
        ir_config_to_compare = IrConfiguration(
            config["data"], config["unit"], config["selector"], ir_config.videoPath)
        if ir_config_to_compare == ir_config:
            return False

    print("Your camera configuration is not in the database shared by the git community.")
    print("When you have finished the setup, could you please take 5 minutes to copy and paste the contents of 'linux-enable-ir-emitter manual' into a new issue: https://github.com/EmixamPP/linux-enable-ir-emitter/issues.")
    print("Thank you for the others !")
    return True


def _config_found(ir_config):
    """Processes the found configuration.

    Args:
        ir_config (IrConfiguration): the config
    """
    if _is_new_config(ir_config):
        with open(config_file_path, "a") as config_file:
            config_file.write("- data: {}\n".format(ir_config.data).replace("'", ""))
            config_file.write("  unit: {}\n".format(ir_config.unit))
            config_file.write("  selector: {}\n".format(ir_config.selector))

    print("Please execute 'linux-enable-ir-emitter quick' to finish the setup.")


def full(video_path):
    """Try to find the ir emitter configuration by capturing the bus.
    On condition that the ir emitter is triggered via a Windows VM.

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    input("Please read and folow this tutorial : https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration. Press enter when you'r ready")
    capture = IrConfigCapture(video_path)
    capture.start(45)
    input("The capturing is finished, make sure the camera is connected to the host os. Press enter when it's done ")

    for ir_config in capture.config_list:
        if _show_config_test(ir_config, False):
            _config_found(ir_config)
            return

    print("No configuration was found", file=sys.stderr)
