http_lib = library('http', [
    'Headers.cpp',
    ],
    dependencies: [
      core_dep,
      ty_dep,
    ])

http_dep = declare_dependency(
  link_with: http_lib,
  include_directories: '..'
  )
