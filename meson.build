project(
    'linux-enable-ir-emitter', 
    'cpp',
    version: '6.0.0', 
    license: 'MIT',
    default_options: [
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
opencv_dep = dependency('opencv4')
gtk_dep = dependency('gtk+-3.0', static: false)
yaml_dep = dependency('yaml-cpp')

##########
# Variable
##########
lib_dir =  get_option('libdir') / meson.project_name()
sysconf_prefix = get_option('prefix') == '/usr' ? '' : get_option('prefix')
cfg_data = configuration_data({
    'version': meson.project_version(),
    'libdir': get_option('prefix') / lib_dir,
    'SAVE_FOLDER_CONFIG_PATH': sysconf_prefix / get_option('sysconfdir') / meson.project_name(),
    'CPP_COMMANDS_LIB_PATH': get_option('prefix') / lib_dir / 'libcommands.so',
    'UDEV_RULE_PATH': sysconf_prefix / get_option('sysconfdir') / 'udev/rules.d/99-linux-enable-ir-emitter.rules',
    'BOOT_SERVICE_MANAGER': get_option('boot_service'),
})
if get_option('boot_service') == 'systemd'
    cfg_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter.service')
elif get_option('boot_service') == 'openrc'
    cfg_data.set('BOOT_SERVICE_NAME', 'linux-enable-ir-emitter')
endif

#####################
# Source to configure
#####################
configure_file(
    input : 'utils/configuration.hpp.in',
    output : 'configuration.hpp',
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
        'utils/configuration.cpp',
        'utils/logger.cpp',
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
install_data(
    'boot_service' / get_option('boot_service') / '__init__.py',
    'boot_service' / get_option('boot_service') / get_option('boot_service') + '.py',
    install_dir : lib_dir / 'boot_service' / get_option('boot_service'),
)

if get_option('boot_service') == 'systemd'
    install_data(
        'boot_service/systemd/linux-enable-ir-emitter.service',
        install_dir : get_option('sysconfdir') / 'systemd/system',
    )
elif get_option('boot_service') == 'openrc'
    install_data(
        'boot_service/openrc/linux-enable-ir-emitter',
        install_dir : get_option('sysconfdir') / 'init.d',
    )
endif

install_emptydir(
    get_option('sysconfdir') / 'udev/rules.d',
)

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
