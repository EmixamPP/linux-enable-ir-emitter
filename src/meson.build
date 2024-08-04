configure_file(
    input: 'utils/configuration.hpp.in',
    output: 'configuration.hpp',
    configuration: cfg_data,
)

configure_file(
    input: 'linux-enable-ir-emitter.cpp.in',
    output: 'linux-enable-ir-emitter.cpp',
    configuration: cfg_data,
)

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
        'utils/configuration.cpp',
    ],
    dependencies: [opencv_dep, gtk_dep, yaml_dep, argparse_dep, spdlog_dep],
    install: true,
    install_dir: get_option('bindir'),
)
