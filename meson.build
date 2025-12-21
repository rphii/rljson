project('rljson', 'c')

sources = [
  'rljson/rljson-core.c',
  'rljson/rljson-auto.c',
  ]

headers = [
  'rljson/rljson-core.h',
  'rljson/rljson-auto.h',
  ]

rlc_dep = dependency('rlc', fallback : ['rlc', 'rlc_dep'], default_options: ['default_library=static'])
rlso_dep = dependency('rlso', fallback : ['rlso', 'rlso_dep'], default_options: ['default_library=static'])

install_headers(headers, subdir: 'rljson')
install_headers('rljson.h')

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


rljson_dep = declare_dependency(
  link_with: librljson,
  include_directories: '.',
  )

if get_option('tests').enabled()
  subdir('tests')
endif

