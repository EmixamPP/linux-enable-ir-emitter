project(
    'linux-enable-ir-emitter', 
    'cpp',
    version: '5.0.3', 
    license: 'MIT',
    default_options: [
        'prefix=/usr',
        'cpp_std=c++17',
        'optimization=3',
        'warning_level=everything',
        'werror=true',
    ],
)

# flag ajustement because of warning_level=everything
add_project_arguments(
    # fix clang-tidy
    '-Wno-unknown-warning-option', '-Wno-unused-macros',
    # ignored bugged or boring gcc flags
    '-Wno-pragmas', '-Wno-abi-tag', '-Wno-suggest-attribute=pure',
    '-Wno-effc++','-Wno-suggest-attribute=const', '-Wno-padded', 
    '-Wformat-nonliteral',  '-Wno-suggest-final-types', '-Wno-inline',
    '-Wno-shadow', '-Wno-missing-declarations', '-Wno-sign-promo', 
    '-Wno-format-nonliteral', '-Wno-suggest-final-methods',
    '-Wno-unused-const-variable',
    language: 'cpp'
) 

opencv_dep = dependency('opencv4', static: true)
gtk_dep = dependency('gtk+-3.0')

################
# Lib executable
################
lib_exec_dir = get_option('libexecdir')  / meson.project_name()

executable(
    'generate-driver', 
    'driver/generate-driver.cpp',
    'camera/camera.cpp',
    'camera/autocamera.cpp',
    'camera/camerainstruction.cpp',
    'driver/driver.cpp',
    'driver/finder.cpp',
    'utils/logger.cpp',
    dependencies: [opencv_dep, gtk_dep],
    install: true,
    install_dir: lib_exec_dir,
)

executable(
    'execute-driver', 
    'driver/execute-driver.cpp',
    'camera/camera.cpp',
    'camera/autocamera.cpp',
    'camera/camerainstruction.cpp',
    'driver/driver.cpp',
    'utils/logger.cpp',
    dependencies: [opencv_dep, gtk_dep],
    install: true,
    install_dir: lib_exec_dir,
)

executable(
    'is-ir-camera', 
    'camera/is-ir-camera.cpp',
    'camera/camera.cpp',
    'camera/camerainstruction.cpp',
    'utils/logger.cpp',
    dependencies: [opencv_dep, gtk_dep],
    install: true,
    install_dir: lib_exec_dir,
)

#####
# Lib
#####
lib_dir = get_option('libdir') / meson.project_name()

configure_version_data = configuration_data({'version': meson.project_version()})
configure_file(
    input : 'linux-enable-ir-emitter.py',
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
    'BIN_IS_CAMERA_PATH': '/usr' / lib_exec_dir / 'is-ir-camera',
    'UDEV_RULE_PATH': '/etc/udev/rules.d/99-linux-enable-ir-emitter.rules',
    'BOOT_SERVICE_MANAGER': boot_service,
})
if boot_service == 'systemd'
    configure_paths_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter.service')
elif boot_service == 'openrc'
    configure_paths_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter')
endif
configure_file(
    input : 'globals.py',
    output : 'globals.py',
    configuration : configure_paths_data,
    install: true,
    install_dir : lib_dir,
)

install_data(
    'boot_service/__init__.py',
    'boot_service/base_boot_service.py',
    install_dir : lib_dir / 'boot_service',
)

install_data(
    'LICENSE',
    install_dir : lib_dir,
)

install_data(
    'command/__init__.py',
    'command/boot.py',
    'command/configure.py',
    'command/delete.py',
    'command/run.py',
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
        'boot_service/systemd/__init__.py',
        'boot_service/systemd/systemd.py',
        install_dir : lib_dir / 'boot_service/systemd',
    )
    install_data(
        'boot_service/systemd/linux-enable-ir-emitter.service',
        install_dir : 'lib/systemd/system',
    )
elif boot_service == 'openrc'
    install_data(
        'boot_service/openrc/__init__.py',
        'boot_service/openrc/openrc.py',
        install_dir : lib_dir / 'boot_service/openrc',
    )
    install_data(
        'boot_service/openrc/linux-enable-ir-emitter',
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
    'autocomplete/linux-enable-ir-emitter',
    install_dir : get_option('datadir') / 'bash-completion/completions',
)
