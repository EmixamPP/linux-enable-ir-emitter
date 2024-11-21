##############
# Dependency #
##############
opencv_dep = dependency('opencv4', include_type: 'system')
gtk_dep = dependency('gtk+-3.0', static: false)
yaml_dep = dependency('yaml-cpp', include_type: 'system')
argparse_dep = dependency('argparse', include_type: 'system')

############
# Variable #
############
cfg_data = configuration_data(
    {
        'version': meson.project_version(),
        'configdir': config_dir,
        'log_file': log_dir / meson.project_name() + '.log',
    },
)

configure_file(
    input: 'linux-enable-ir-emitter.cpp.in',
    output: 'linux-enable-ir-emitter.cpp',
    configuration: cfg_data,
)

configure_file(
    input: 'configuration/configuration.cpp.in',
    output: 'configuration.cpp',
    configuration: cfg_data,
)

configure_file(
    input: 'utils/_logger.hpp.in',
    output: '_logger.hpp',
    configuration: cfg_data,
)

##########
# Source #
##########
executable(
    'linux-enable-ir-emitter',
    sources: [
        meson.current_build_dir() / 'linux-enable-ir-emitter.cpp',
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
        meson.current_build_dir() / 'configuration.cpp',
    ],
    dependencies: [opencv_dep, gtk_dep, yaml_dep, argparse_dep],
    install: true,
    install_dir: get_option('bindir'),
)

#########
# Tests #
#########
if get_option('tests')
    gtest_dep = dependency('gtest', static: false)
    test_exe = executable(
        'linux-enable-ir-emitter-test',
        sources: [
            '../tests/main.test.cpp',
            '../tests/camerainstruction.test.cpp',
            'camera/camera.cpp',
            'camera/camerainstruction.cpp',
        ],
        dependencies: [gtest_dep, opencv_dep, gtk_dep, yaml_dep],
        install: false,
    )
    test(
        'linux-enable-ir-emitter', 
        test_exe, 
        timeout: 0
    )
endif