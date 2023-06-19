project(
    'linux-enable-ir-emitter', 
    'cpp',
    version: 'unsupported', 
    license: 'MIT',
    default_options: [
        'cpp_std=c++17',
        'optimization=3',
        'warning_level=3',
        'prefix=/usr',
    ], 
)

opencv_dep = dependency('opencv4', static: true)

version = 'unsupported'
lib_dir = get_option('libdir') / meson.project_name()
lib_bin_dir = lib_dir / 'bin'
sysconf_dir = get_option('sysconfdir')
openrc_dir = '/etc/init.d'
autocomplete_dir = get_option('datadir') / 'bash-completion/completions'


executable(
    'driver-generator', 
    'sources/driver/driver-generator.cpp',
    'sources/driver/camera.cpp',
    'sources/driver/logger.cpp',
    'sources/driver/driver.cpp',
    'sources/driver/finder.cpp',
    dependencies: opencv_dep,
    install: true,
    install_dir: lib_bin_dir,
)

executable(
    'execute-driver', 
    'sources/driver/execute-driver.cpp',
    'sources/driver/camera.cpp',
    'sources/driver/logger.cpp',
    'sources/driver/driver.cpp',
    dependencies: opencv_dep,
    install: true,
    install_dir: lib_bin_dir,
)

configure_version_data = configuration_data({'version': version})
configure_file(
    input : 'sources/linux-enable-ir-emitter.py.in',
    output : 'linux-enable-ir-emitter.py',
    configuration : configure_version_data,
    install: true,
    install_dir : lib_dir,
    install_mode: 'rwxr-xr-x',
)

install_symlink(
    meson.project_name(),
    pointing_to: '../' / lib_dir / 'linux-enable-ir-emitter.py',
    install_dir: get_option('bindir'),
)

configure_paths_data = configuration_data({
    'SAVE_DRIVER_FOLDER_PATH': sysconf_dir / meson.project_name(),
    'BIN_EXECUTE_DRIVER_PATH': '/usr' / lib_bin_dir / 'execute-driver',
    'BIN_DRIVER_GENERATOR_PATH': '/usr' / lib_bin_dir / 'driver-generator',
    'UDEV_RULE_PATH': '/etc/udev/rules.d/99-linux-enable-ir-emitter.rules',
    'BOOT_SERVICE_NAME': 'linux-enable-ir-emitter',
    'BOOT_SERVICE_MANAGER': 'openrc'
})
configure_file(
    input : 'sources/globals.py.in',
    output : 'globals.py',
    configuration : configure_paths_data,
    install: true,
    install_dir : lib_dir,
)

install_data(
    'LICENSE',
    'README.md',
    install_dir : lib_dir,
)

install_data(
    'sources/command/__init__.py',
    'sources/command/boot.py',
    'sources/command/configure.py',
    'sources/command/delete.py',
    'sources/command/run.py',
    install_dir : lib_dir / 'command',
)

install_subdir(
    meson.project_name(),
    install_dir: sysconf_dir,
)

install_data(
    'sources/boot_service/__init__.py',
    'sources/boot_service/base_boot_service.py',
    install_dir : lib_dir / 'boot_service',
)

install_data(
    'sources/boot_service/openrc/__init__.py',
    'sources/boot_service/openrc/openrc.py',
    install_dir : lib_dir / 'boot_service/openrc',
)

install_data(
    'sources/boot_service/openrc/linux-enable-ir-emitter',
    install_dir : openrc_dir,
)

install_data(
    'sources/autocomplete/linux-enable-ir-emitter',
    install_dir : autocomplete_dir,
)
