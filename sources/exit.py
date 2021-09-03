import sys
import enum

class code(enum.IntEnum):
    SUCCESS = 0
    FAILURE = 1
    FILE_DESCRIPTOR_ERROR = 2
    MISSING_DEPENDENCY = 3
    PERMISSION_DENIED = 126
    

def exit(exit_code):
    """Exit the script

    Args:
        exit_code (code): code to return
    """
    sys.exit(exit_code)