html_lib = library('html', [
    'Headers.cpp',
    ],
    dependencies: [
      core_dep,
      ty_dep,
    ])

html_dep = declare_dependency(
  link_with: html_lib,
  include_directories: '..'
  )
