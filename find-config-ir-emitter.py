#!/usr/bin/env python

import os
from yaml import load
from os import system
from sys import argv


if __name__ == "__main__":
    with open(argv[1]) as config_file:
        config_list = load(config_file)

    script_path = os.path.dirname(os.path.abspath(__file__))

    for config in config_list:
        data = config["data"]
        data_size = data.count(" ") + 1
        unit = config["unit"]
        selector = config["selector"]
        params = "-dataSize {} -data {} -unit {} -selector {}".format(data_size, data, unit, selector)
        command = f"{script_path}/enable-ir-emitter {params}"
        res = system(command)

        if not res:
            print(params)
            exit(0)

    print("No configuration has been found.")
    exit(1)
