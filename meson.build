project(
    'linux-enable-ir-emitter',
    'cpp',
    version: '6.0.6-dev',
    license: 'MIT',
    default_options: [
        'cpp_std=c++20',
        'buildtype=release',
        'warning_level=everything',
    ],
)

# flag ajustement because of warning_level=everything
add_project_arguments(
    # fix clang-tidy
    '-Wno-unknown-warning-option',
    # impossible to solve here
    '-Wno-padded',
    # without importance
    '-Wno-shadow',
    # unworth increase of reading complexity
    '-Wno-abi-tag', 
    '-Wno-effc++',
    '-Wno-suggest-attribute=pure',
    # let the compiler tries
    '-Wno-inline',
    # raised by stl
    '-Wno-alloc-zero',
    '-Wno-strict-overflow',
    # bugged
    '-Wno-unused-const-variable',
    '-Wno-suggest-attribute=const',
    '-Wno-suggest-final-types',
    '-Wno-suggest-final-methods',
    language: 'cpp',
)

if get_option('prefer_static')
    add_project_link_arguments(
        '-static-libgcc',
        '-static-libstdc++',
        language: 'cpp',
    )
endif

############
# Variable #
############
config_dir = get_option('sysconfdir') / meson.project_name()
log_dir = get_option('localstatedir') / 'log' / meson.project_name()

#####################
# Executable + Test #
#####################
subdir('src')

################
# Config + Log #
################
install_emptydir(config_dir)
install_emptydir(log_dir)

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
        install_dir: get_option('sysconfdir') / 'systemd/system',
    )
elif get_option('boot_service') == 'openrc'
    install_data(
        'boot_service/openrc/linux-enable-ir-emitter',
        install_dir: get_option('sysconfdir') / 'init.d',
    )
endif

##########
# Others #
##########
install_data(
    'README.md',
    install_dir: get_option('datadir') / 'doc' / meson.project_name(),
)

install_data(
    'LICENSE',
    install_dir: get_option('datadir') / 'doc' / meson.project_name(),
)

###############
# Extra Tests #
###############
if get_option('tests')
    subdir('tests')
endif
