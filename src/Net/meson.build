net_lib = library('net', [
    'TCPConnection.cpp',
    'TCPListener.cpp',
  ],
  dependencies: [
    core_dep,
    ty_dep,
  ])

net_dep = declare_dependency(
  link_with: net_lib,
  include_directories: '..'
  )

