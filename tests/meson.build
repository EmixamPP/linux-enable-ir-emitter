meson_exe = find_program('meson')

test(
    'clang-tidy',
    meson_exe, 
    args: [
        '--internal',
        'clangtidy',
        meson.project_source_root(),
        meson.project_build_root()
    ],
    timeout: 0,
)

test(
    'clang-format',
    meson_exe, 
    args: [
        '--internal',
        'clangformat',
        '--check',
        meson.project_source_root(),
        meson.project_build_root()
    ],
    timeout: 0,
)

gtest_dep = dependency('gtest', static: false)
test_exe = executable(
    'linux-enable-ir-emitter-test',
    sources: [
        'main.test.cpp',
        'camerainstruction.test.cpp',
        '../src/camera/camera.cpp',
        '../src/camera/camerainstruction.cpp',
    ],
    dependencies: [gtest_dep, opencv_dep, gtk_dep, yaml_dep],
    install: false,
)
test(
    'linux-enable-ir-emitter', 
    test_exe, 
    timeout: 0
)
