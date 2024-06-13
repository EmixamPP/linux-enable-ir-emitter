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

##############
# Dependency #
##############
opencv_dep = dependency('opencv4', include_type: 'system')
gtk_dep = dependency('gtk+-3.0', static: false)
yaml_dep = dependency('yaml-cpp', include_type: 'system')
argparse_dep = dependency('argparse', include_type: 'system')
spdlog_dep = dependency('spdlog', include_type: 'system')

############
# Variable #
############
sysconf_dir = '/' / get_option('sysconfdir') / meson.project_name()
log_dir = get_option('localstatedir') / 'log' / meson.project_name()
cfg_data = configuration_data({
    'version': meson.project_version(),
    'configdir': sysconf_dir,
    'log_file': log_dir / meson.project_name() + '.log',
})

##############
# Executable #
##############
configure_file(
    input : 'utils/configuration.hpp.in',
    output : 'configuration.hpp',
    configuration : cfg_data,
)

configure_file(
    input : 'linux-enable-ir-emitter.cpp.in',
    output : 'linux-enable-ir-emitter.cpp',
    configuration : cfg_data,
)

executable(
    'linux-enable-ir-emitter',
    sources: [
        meson.project_build_root() / 'linux-enable-ir-emitter.cpp',
        'camera/autocamera.cpp',
        'camera/camera.cpp',
        'camera/camerainstruction.cpp',
        'command/configure.cpp',
        'command/run.cpp',
        'command/test.cpp',
        'command/tweak.cpp',
        'configuration/finder.cpp',
        'configuration/scanner.cpp',
        'configuration/tweaker.cpp',
        'utils/configuration.cpp',
    ],
    dependencies: [opencv_dep, gtk_dep, yaml_dep, argparse_dep, spdlog_dep],
    install: true,
    install_dir: get_option('bindir')
)

#################
# Configuration #
#################
install_emptydir(sysconf_dir)

####################
# Shell completion #
####################
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

################
# Boot service #
################
if get_option('boot_service') == 'systemd'
    install_data(
        'boot_service/systemd/linux-enable-ir-emitter.service',
        install_dir : '/' / get_option('sysconfdir') / 'systemd/system',
    )
elif get_option('boot_service') == 'openrc'
    install_data(
        'boot_service/openrc/linux-enable-ir-emitter',
        install_dir : '/' / get_option('sysconfdir') / 'init.d',
    )
endif

##########
# Others #
##########
install_emptydir(log_dir)

install_data(
    'LICENSE',
    install_dir : get_option('datadir') / 'licences' / meson.project_name(),
)
