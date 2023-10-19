import ctypes

from utils import CPP_COMMANDS_LIB_PATH

cpp_commands = ctypes.CDLL(CPP_COMMANDS_LIB_PATH)

cpp_commands.configure.argtypes = [
    ctypes.c_char_p,
    ctypes.c_bool,
    ctypes.c_uint32,
    ctypes.c_uint32,
]
cpp_commands.configure.restype = ctypes.c_int

cpp_commands.delete_driver.argtypes = [ctypes.c_char_p]
cpp_commands.delete_driver.restype = ctypes.c_int

cpp_commands.run.argtypes = [ctypes.c_char_p]
cpp_commands.run.restype = ctypes.c_int

cpp_commands.test.argtypes = [ctypes.c_char_p]
cpp_commands.test.restype = ctypes.c_int
