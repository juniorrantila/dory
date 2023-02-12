subdir('Ty')
subdir('Core')
subdir('Mem')
subdir('CLI')
subdir('Main')
subdir('HTTP')
subdir('Net')
subdir('Web')

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
    net_dep,
    http_dep,
    web_dep,
  ])
