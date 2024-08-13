project(
    'linux-enable-ir-emitter',
    'cpp',
    version: '6.0.2',
    license: 'MIT',
    default_options: [
        'cpp_std=c++20',
        'buildtype=release',
        'warning_level=everything',
        'werror=true',
    ],
)

# flag ajustement because of warning_level=everything
add_project_arguments(
    '-Wno-unknown-warning-option', # fix clang-tidy
    '-Wno-padded', # impossible to solve here
    '-Wno-shadow', # without importance
    '-Wno-abi-tag', # unworth increase of reading complexity
    '-Wno-effc++', # unworth increase of reading complexity
    '-Wno-unused-const-variable', # bugged
    '-Wno-suggest-attribute=const', # bugged
    '-Wno-inline', # let the compiler tries
    '-Wno-suggest-attribute=pure', # unworth increase of reading complexity
    '-Wno-suggest-final-types', # bugged
    '-Wno-suggest-final-methods', # bugged
    language: 'cpp',
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
cfg_data = configuration_data(
    {
        'version': meson.project_version(),
        'configdir': sysconf_dir,
        'log_file': log_dir / meson.project_name() + '.log',
    },
)

##############
# Executable #
##############
subdir('src')

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
    install_dir: get_option('datadir') / 'bash-completion/completions',
)

install_data(
    'autocomplete/linux-enable-ir-emitter.zsh',
    rename: '_linux-enable-ir-emitter',
    install_dir: get_option('datadir') / 'zsh/site-functions',
)

################
# Boot service #
################
if get_option('boot_service') == 'systemd'
    install_data(
        'boot_service/systemd/linux-enable-ir-emitter.service',
        install_dir: '/' / get_option('sysconfdir') / 'systemd/system',
    )
elif get_option('boot_service') == 'openrc'
    install_data(
        'boot_service/openrc/linux-enable-ir-emitter',
        install_dir: '/' / get_option('sysconfdir') / 'init.d',
    )
endif

##########
# Others #
##########
install_emptydir(log_dir)

install_data(
    'LICENSE',
    install_dir: get_option('datadir') / 'licences' / meson.project_name(),
)
