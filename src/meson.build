subdir('Ty')
subdir('Core')
subdir('Mem')
subdir('CLI')
subdir('Main')
subdir('HTTP')

nemo_exe = executable('nemo', [
    'main.cpp',
  ],
  include_directories: '.',
  dependencies: [
    cli_dep,
    core_dep,
    main_dep,
    mem_dep,
    ty_dep,
    http_dep,
  ])
