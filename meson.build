project(
    'linux-enable-ir-emitter', 
    'cpp',
    version: '6.0.0', 
    license: 'MIT',
    default_options: [
        'prefix=/usr',
        'cpp_std=c++17',
        'buildtype=release',
        'warning_level=everything',
        'werror=true',
    ],
)

# flag ajustement because of warning_level=everything
add_project_arguments(
    # fix clang-tidy
    '-Wno-unknown-warning-option', '-Wno-unused-macros',
    # ignore bugged and boring flags
    '-Wno-pragmas', '-Wno-abi-tag', '-Wno-effc++',
    '-Wno-padded', '-Wno-shadow', '-Wno-sign-promo', 
    '-Wno-unused-const-variable', '-Wno-suggest-final-types',
    '-Wno-suggest-final-methods', '-Wno-suggest-attribute=pure',
    '-Wno-suggest-attribute=const', '-Wno-inline',
    language: 'cpp'
)

############
# Dependency
############
static_dep = get_option('static_dep')
opencv_dep = dependency('opencv4', static: static_dep)
gtk_dep = dependency('gtk+-3.0')
yaml_dep = dependency('yaml-cpp', static: static_dep)

##########
# Variable
##########
lib_dir = get_option('libdir') / meson.project_name()
abs_lib_dir = get_option('prefix') / lib_dir
boot_service = get_option('boot_service')
cfg_data = configuration_data({
    'version': meson.project_version(),
    'libdir': abs_lib_dir,
    'SAVE_FOLDER_CONFIG_PATH': get_option('sysconfdir') / meson.project_name(),
    'CPP_COMMANDS_LIB_PATH': abs_lib_dir / 'libcommands.so',
    'UDEV_RULE_PATH': '/etc/udev/rules.d/99-linux-enable-ir-emitter.rules',
    'BOOT_SERVICE_MANAGER': boot_service,
})
if boot_service == 'systemd'
    cfg_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter.service')
elif boot_service == 'openrc'
    cfg_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter')
endif

#####################
# Source to configure
#####################
configure_file(
    input : 'utils/globals.hpp.in',
    output : 'globals.hpp',
    configuration : cfg_data,
)

##################
# Bin to configure
##################
configure_file(
    input : 'linux-enable-ir-emitter.py',
    output : 'linux-enable-ir-emitter',
    configuration : cfg_data,
    install: true,
    install_dir : get_option('bindir') ,
    install_mode: 'rwxr-xr-x',
)

##################
# Lib to configure
##################
configure_file(
    input : 'utils/globals.py',
    output : 'globals.py',
    configuration : cfg_data,
    install: true,
    install_dir : lib_dir / 'utils',
)

#####
# Lib
#####
library(
    'commands',
    sources: [
        'camera/autocamera.cpp',
        'camera/camera.cpp',
        'camera/camerainstruction.cpp',
        'command/configure.cpp',
        'command/delete.cpp',
        'command/run.cpp',
        'command/test.cpp',
        'command/tweak.cpp',
        'configuration/finder.cpp',
        'configuration/scanner.cpp',
        'configuration/tweaker.cpp',
        'utils/globals.cpp',
        'utils/logger.cpp',
        'utils/configuration.cpp',
    ],
    dependencies: [opencv_dep, gtk_dep, yaml_dep],
    install: true,
    install_dir: lib_dir
)

install_data(
    'utils/__init__.py',
    install_dir : lib_dir / 'utils',
)

install_data(
    'command/__init__.py',
    'command/boot.py',
    'command/load_cpp_commands.py',
    install_dir : lib_dir / 'command',
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
install_emptydir(
    get_option('sysconfdir') / meson.project_name(),
)

##################
# Shell completion
##################
install_data(
    'autocomplete/linux-enable-ir-emitter.bash',
    rename: 'linux-enable-ir-emitter',
    install_dir : get_option('datadir') / 'bash-completion/completions',
)
install_data(
    'autocomplete/linux-enable-ir-emitter.zsh',
    rename: '_linux-enable-ir-emitter',
    install_dir : get_option('datadir') / 'zsh/site-functions',
)
