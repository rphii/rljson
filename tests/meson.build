
cur_src = meson.current_source_dir()

dep_rlc = dependency('rlc', required: true)
dep_rlso = dependency('rlso', required: true)

ex = executable('test_rljson_exe', 'test.c', link_with: librljson, dependencies: [dep_rlc, dep_rlso])

pass_tests_non_strict = [
  'data/pass1.json',
  'data/pass2.json',
  'data/pass3.json',
  'data/fail1.json',
  'data/fail18.json',
  'data/fail25.json',
  ]

fail_tests_non_strict = [
  'data/fail2.json',
  'data/fail3.json',
  'data/fail4.json',
  'data/fail5.json',
  'data/fail6.json',
  'data/fail7.json',
  'data/fail8.json',
  'data/fail9.json',
  'data/fail10.json',
  'data/fail11.json',
  'data/fail12.json',
  'data/fail13.json',
  'data/fail14.json',
  'data/fail15.json',
  'data/fail16.json',
  'data/fail17.json',
  'data/fail19.json',
  'data/fail20.json',
  'data/fail21.json',
  'data/fail22.json',
  'data/fail23.json',
  'data/fail24.json',
  'data/fail26.json',
  'data/fail27.json',
  'data/fail28.json',
  'data/fail29.json',
  'data/fail30.json',
  'data/fail31.json',
  'data/fail32.json',
  'data/fail33.json',
  ]

fail_tests_strict = [
  'data/fail1.json',
  'data/fail25.json',
  ]

foreach file : pass_tests_non_strict
  test('non-strict / pass / ' + file, ex, args: ['pass', join_paths(cur_src, file), 'non-strict'])
endforeach

foreach file : fail_tests_non_strict
  test('non-strict / fail / ' + file, ex, args: ['fail', join_paths(cur_src, file), 'non-strict'])
endforeach

foreach file : fail_tests_strict
  test('strict     / fail / ' + file, ex, args: ['fail', join_paths(cur_src, file), 'strict'])
endforeach

