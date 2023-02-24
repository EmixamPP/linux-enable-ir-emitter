project(
    'linux-enable-ir-emitter', 
    'cpp',
    version: '4.4.0', 
    license: 'MIT',
    default_options: [
        'cpp_std=c++17',
        'optimization=3',
        'warning_level=3',
        'prefix=/usr',
    ], 
)

opencv_dep = dependency('opencv4', static: true)

executable(
    'driver-generator', 
    'sources/driver/driver-generator.cpp',
    'sources/driver/camera.cpp',
    'sources/driver/logger.cpp',
    'sources/driver/driver.cpp',
    'sources/driver/finder.cpp',
    dependencies: opencv_dep,
    install: true,
    install_dir: get_option('libdir') / meson.project_name() / 'bin',
)

executable(
    'execute-driver', 
    'sources/driver/execute-driver.cpp',
    'sources/driver/camera.cpp',
    'sources/driver/logger.cpp',
    'sources/driver/driver.cpp',
    dependencies: opencv_dep,
    install: true,
    install_dir: get_option('libdir') / meson.project_name() / 'bin',
)

install_data(
    'LICENSE',
    'README.md',
    'sources/globals.py', 
    install_dir : get_option('libdir') / meson.project_name(),
)

install_data(
    'sources/linux-enable-ir-emitter.py',
    install_dir : get_option('libdir') / meson.project_name(),
    install_mode: 'rwxr-xr-x',
)

install_symlink(
    meson.project_name(),
    pointing_to: '../' / get_option('libdir') / meson.project_name() / 'linux-enable-ir-emitter.py',
    install_dir: get_option('bindir'),
)

install_data(
    'sources/command/__init__.py',
    'sources/command/boot.py',
    'sources/command/configure.py',
    'sources/command/delete.py',
    'sources/command/run.py',
    install_dir : get_option('libdir') / meson.project_name() / 'command',
)

install_data(
    'sources/systemd/__init__.py',
    'sources/systemd/Systemd.py',
    install_dir : get_option('libdir') / meson.project_name() / 'systemd',
)

install_data(
    'sources/systemd/linux-enable-ir-emitter.service',
    install_dir : 'lib/systemd/system',
)

install_subdir(
    meson.project_name(),
    install_dir: get_option('sysconfdir'),
)

install_data(
    'sources/autocomplete/linux-enable-ir-emitter',
    install_dir : get_option('datadir') / 'bash-completion/completions',
)
