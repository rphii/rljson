sources = [
  'json.c',
  ]

headers = [
  'json.h',
  ]

dep_rphiic = dependency('rphiic', required: true)
dep_rphiiso = dependency('rphiiso', required: true)

install_headers(headers, subdir: 'rphii')
rphiijson = library('rphiijson',
  sources,
  dependencies: [dep_rphiic, dep_rphiiso],
  install: true,
  )




