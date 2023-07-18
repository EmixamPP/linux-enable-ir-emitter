project(
    'linux-enable-ir-emitter', 
    'cpp',
    version: '5.0.0-pre', 
    license: 'MIT',
    default_options: [
        'prefix=/usr',
        'cpp_std=c++17',
        'optimization=3',
        'warning_level=everything',
        'werror=true',
    ],
)
add_project_arguments(
    '-Wno-pragmas', '-Wno-abi-tag', '-Wno-suggest-attribute=pure',
    '-Wno-suggest-attribute=const', '-Wno-padded', '-Wformat-nonliteral',
    '-Wno-suggest-final-types', '-Wno-suggest-final-methods', '-Wno-shadow',
    '-Wno-missing-declarations', '-Wno-sign-promo', '-Wno-format-nonliteral',
    language: 'cpp'
) 

opencv_dep = dependency('opencv4', static: true)

################
# Lib executable
################
lib_exec_dir = get_option('libexecdir')  / meson.project_name()

executable(
    'generate-driver', 
    'sources/driver/generate-driver.cpp',
    'sources/camera/camera.cpp',
    'sources/camera/autocamera.cpp',
    'sources/camera/camerainstruction.cpp',
    'sources/driver/driver.cpp',
    'sources/driver/finder.cpp',
    'sources/driver/exhaustivefinder.cpp',
    'sources/utils/logger.cpp',
    dependencies: opencv_dep,
    install: true,
    install_dir: lib_exec_dir,
)

executable(
    'execute-driver', 
    'sources/driver/execute-driver.cpp',
    'sources/camera/camera.cpp',
    'sources/camera/autocamera.cpp',
    'sources/camera/camerainstruction.cpp',
    'sources/driver/driver.cpp',
    'sources/utils/logger.cpp',
    dependencies: opencv_dep,
    install: true,
    install_dir: lib_exec_dir,
)

#####
# Lib
#####
lib_dir = get_option('libdir') / meson.project_name()

configure_version_data = configuration_data({'version': meson.project_version()})
configure_file(
    input : 'sources/linux-enable-ir-emitter.py',
    output : 'linux-enable-ir-emitter.py',
    configuration : configure_version_data,
    install: true,
    install_dir : lib_dir,
    install_mode: 'rwxr-xr-x',
)

boot_service = get_option('boot_service')

configure_paths_data = configuration_data({
    'SAVE_DRIVER_FOLDER_PATH': get_option('sysconfdir') / meson.project_name(),
    'BIN_EXECUTE_DRIVER_PATH': '/usr' / lib_exec_dir / 'execute-driver',
    'BIN_GENERATE_DRIVER_PATH': '/usr' / lib_exec_dir / 'generate-driver',
    'UDEV_RULE_PATH': '/etc/udev/rules.d/99-linux-enable-ir-emitter.rules',
    'BOOT_SERVICE_MANAGER': boot_service,
})
if boot_service == 'systemd'
    configure_paths_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter.service')
elif boot_service == 'openrc'
    configure_paths_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter')
endif
configure_file(
    input : 'sources/globals.py',
    output : 'globals.py',
    configuration : configure_paths_data,
    install: true,
    install_dir : lib_dir,
)

install_data(
    'sources/boot_service/__init__.py',
    'sources/boot_service/base_boot_service.py',
    install_dir : lib_dir / 'boot_service',
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

############
# Executable
############
install_symlink(
    meson.project_name(),
    pointing_to: '../' / lib_dir / 'linux-enable-ir-emitter.py',
    install_dir: get_option('bindir'),
)

##############
# Boot service
##############
if boot_service == 'systemd'
    install_data(
        'sources/boot_service/systemd/__init__.py',
        'sources/boot_service/systemd/systemd.py',
        install_dir : lib_dir / 'boot_service/systemd',
    )
    install_data(
        'sources/boot_service/systemd/linux-enable-ir-emitter.service',
        install_dir : 'lib/systemd/system',
    )
elif boot_service == 'openrc'
    install_data(
        'sources/boot_service/openrc/__init__.py',
        'sources/boot_service/openrc/openrc.py',
        install_dir : lib_dir / 'boot_service/openrc',
    )
    install_data(
        'sources/boot_service/openrc/linux-enable-ir-emitter',
        install_dir : '/etc/init.d',
    )
endif

######
# Conf
######
install_subdir(
    meson.project_name(),
    install_dir: get_option('sysconfdir'),
)

##################
# Shell completion
##################
install_data(
    'sources/autocomplete/linux-enable-ir-emitter',
    install_dir : get_option('datadir') / 'bash-completion/completions',
)
