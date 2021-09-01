import yaml
import os
import sys

from IrConfiguration import IrConfiguration

local_path = path = os.path.dirname(os.path.abspath(__file__))
config_file_path = local_path + "/config.yaml"
save_config_file_path = local_path + "/irConfig.yaml"

systemd_name = "linux-enable-ir-emitter.service"
systemd_file_path = "/usr/lib/systemd/system/" + systemd_name


def _load_saved_config():
    """Load the ir config saved.

    Returns:
        IrConfiguration: the saved config
        None: if a error occur
    """
    try:
        if os.path.exists(save_config_file_path):
            return IrConfiguration.load(save_config_file_path)
        else:
            print("No configuration is currently saved.", file=sys.stderr)
    except:
        print("The config file is corrupted !", file=sys.stderr)
        print("Execute 'linux-enable-ir-emitter fix config' to reset the file.", file=sys.stderr)
    return None


def run():
    """Run the config saved in irConfig.yaml"""
    ir_config = _load_saved_config()
    if ir_config:
        ir_config.run()


def test():
    """Try to trigger the ir emitter with the current configuration"""
    ir_config = _load_saved_config()
    if ir_config:
        ir_config.trigger_ir(2)


def boot(status):
    """Enable or disable the systemd service which activates the ir emitter

    Args:
        status (string): "enable" or "disable" or "status"

    Raises:
        Exception: boot status arg can only be equal to enable, disable or status
    """
    if status in ("enable", "disable", "status"):
        os.system("systemctl {} --now {}".format(status, systemd_name))
    else:
        raise Exception("boot status arg can only be equal to 'enable', 'disable' or 'status'")


def manual(video_path):
    """Display the current configuration in the nano editor

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    dummy_config = IrConfiguration([0], 0, 0, video_path)
    if not os.path.exists(save_config_file_path):
        dummy_config.save(save_config_file_path)

    os.system("/bin/nano " + save_config_file_path)
    if _load_saved_config() == dummy_config:
        os.system("rm " + save_config_file_path)


def _show_config_test(ir_config):
    """Test the configuration and ask the user if it works. 
    In this case the configuration is saved and a help message is displayed with what the user can do. 

    Args:
        ir_config (IrConfiguration): configuration to test

    Returns:
        bool: True if the configuration works, else False
    """
    if not ir_config.run():
        ir_config.trigger_ir()

        check = input("Did you see the ir emitter flashing ? Yes/No ? ").lower()
        while (check not in ("yes", "y", "no", "n")):
            check = input("Yes/No ? ").lower()

        if check in ("yes", "y"):
            ir_config.save(save_config_file_path)
            print("A configuration have been found. Here is what you can do:")
            print("  - activate the emitter at system boot : 'linux-enable-ir-emitter boot enable'")
            print("  - manually activate the emitter for one session : 'linux-enable-ir-emitter run'")
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
        ir_config = IrConfiguration(config["data"], config["unit"], config["selector"], video_path)
        if _show_config_test(ir_config):
            return

    print("No configuration was found please execute : 'linux-enable-ir-emitter full'.", file=sys.stderr)


def _show_contribution(ir_config):
    """Check if the configuration does not exist is not in the configuration file. 
    Then the user is invited to share it on Github.

    Args:
        ir_config (IrConfiguration): the configuration to be compared with those in the YAML config file
    """
    with open(config_file_path, "r") as config_file:
        config_list = yaml.load(config_file, Loader=yaml.Loader)

    for config in config_list:
        ir_config_to_compare = IrConfiguration(config["data"], config["unit"], config["selector"], ir_config.videoPath)
        if ir_config_to_compare == ir_config:
            return

    print("Your camera configuration is not in the database shared by the git community.")
    print("Could you please take 5 minutes to paste the contents of 'linux-enable-ir-emitter manual' into a new issue: https://github.com/EmixamPP/linux-enable-ir-emitter/issues.")
    print("Thank you for the others !")


def full(video_path):
    """Try to find the ir emitter configuration by capturing the bus.
    On condition that the ir emitter is triggered via a Windows VM.
    Exit if pysharsk is not installed.

    Args:
        video_path (string): Path to the infrared camera e.g : "/dev/video2"
    """
    try:
        from IrConfigCapture import IrConfigCapture
    except ImportError:
        print("The 'pyshark' python dependency is required for this command.", file=sys.stderr)
        print("Please consult https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration.")
        sys.exit(1)

    input("Please read and folow this tutorial: https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Semi-automatic-configuration. Press enter when you'r ready ")
    capture = IrConfigCapture(video_path)
    capture.start(45)
    input("The capturing is finished, make sure the camera is connected to the host os. Press enter when it's done ")

    for ir_config in capture.config_list:
        if _show_config_test(ir_config):
            _show_contribution(ir_config)
            return
    print("No configuration was found.", file=sys.stderr)


def fix(target):
    """Fix well know problems

    Args:
        target (string): "config" or "chicony"

    Raises:
        Exception: fix target arg can only be equal to config or chicony
    """
    if target in ("config", "chicony"):
        eval("_fix_" + target + "()")
    else:
        raise Exception("fix target arg can only be equal to 'config' or 'chicony'")


def _fix_config():
    """Rest the configuration"""
    os.system("rm -f " + save_config_file_path)
    print("The configuration file have been removed.")


def _fix_chicony():
    """Uninstall chicony-ir-toggle"""
    os.system("rm -f /usr/local/bin/chicony-ir-toggle")
    os.system("rm -f /lib/udev/rules.d/99-ir-led.rules")
    os.system("rm -f /lib/systemd/system-sleep/ir-led.sh")
    print("chicony-ir-toggle have been uninstall.")
