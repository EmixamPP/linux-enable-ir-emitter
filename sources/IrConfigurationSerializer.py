import os
import yaml
import sys
import logging

from globals import SAVE_CONFIG_FILE_PATH, ExitCode
from IrConfiguration import IrConfiguration


class IrConfigurationSerializer:
    @staticmethod
    def _deserialize_all_saved_config():
        """Load all ir configurations saved in globals.SAVE_CONFIG_FILE_PATH
        No error catching

        Returns:
            IrConfiguration list: saved configurations
        """
        with open(SAVE_CONFIG_FILE_PATH, "r") as save_file:
            return list(yaml.load_all(save_file, Loader=yaml.Loader))

    @staticmethod
    def load_all_saved_config():
        """Load all ir configurations saved in globals.SAVE_CONFIG_FILE_PATH

        Returns:
            IrConfiguration list: saved configurations
            None: if no configuration is saved
        """
        try:
            if os.path.exists(SAVE_CONFIG_FILE_PATH):
                dummy_config = IrConfiguration([0], 0, 0, '')
                deserialized = IrConfigurationSerializer._deserialize_all_saved_config()
                for config in deserialized:
                    assert(dir(dummy_config) == dir(config))
                return deserialized
            logging.error("No configuration is currently saved.")
        except:
            logging.critical("The configuration file is corrupted.")
            logging.info("Execute 'linux-enable-ir-emitter fix config' to reset the file.")
            sys.exit(ExitCode.FAILURE)
    
    @staticmethod
    def save_all_config(config_list):
        """Save all configurations in globals.SAVE_CONFIG_FILE_PATH

        Args:
            config_list (IrConfiguration list): configurations to save
        """
        with open(SAVE_CONFIG_FILE_PATH, "w") as save_config_file:
            save_config_file.write("#Caution: any manual modification of this file may corrupt the operation of the program! You must therefore be very careful.\n")
            save_config_file.write("#Please consult https://github.com/EmixamPP/linux-enable-ir-emitter/wiki/Manual-configuration before.\n")
            save_config_file.write("#If you currupt the config file: execute 'linux-enable-ir-emitter fix config' to reset the file.\n\n")
            yaml.dump_all(config_list, save_config_file)
    
    @staticmethod
    def add_config(config):
        """Add a configuration to file globals.SAVE_CONFIG_FILE_PATH

        Args:
            config (IrConfiguration): configuration to add
        """
        saved_config_list = None
        try:
            saved_config_list = IrConfigurationSerializer._deserialize_all_saved_config()
            for saved_config in saved_config_list.copy():
                if saved_config.device == config.device:
                    saved_config_list.remove(saved_config)
            saved_config_list.append(config)
        except:  # if config file corrupted or no config saved: delete all saved configuration
            saved_config_list = [config]
        IrConfigurationSerializer.save_all_config(saved_config_list)