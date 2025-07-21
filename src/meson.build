sources = [
  'json.c',
  ]

headers = [
  'json.h',
  ]

dep_rlc = dependency('rlc', required: true)
dep_rlso = dependency('rlso', required: true)

install_headers(headers, subdir: 'rl')
rljson = library('rljson',
  sources,
  dependencies: [dep_rlc, dep_rlso],
  install: true,
  )




