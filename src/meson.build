sources = [
  'rljson.c',
  ]

headers = [
  ]

rlso_dep = dependency('rlso', fallback : ['rlso', 'rlso_dep'], default_options: ['default_library=static'])
rlc_dep = dependency('rlc', fallback : ['rlc', 'rlc_dep'], default_options: ['default_library=static'])

install_headers('rljson.h')
install_headers(headers, subdir: 'rl')

librljson = library('rljson',
  sources,
  dependencies: [rlc_dep, rlso_dep],
  install: true,
  )

pkg = import('pkgconfig')
pkg.generate(
  libraries: librljson,
  version: '9999',
  name: 'rljson',
  description: 'Immediate JSON verifier, parser in C',
  )

rlarg_dep = declare_dependency(
  link_with: librljson,
  include_directories: '.',
  )



