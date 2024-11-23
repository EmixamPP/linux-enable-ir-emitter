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

