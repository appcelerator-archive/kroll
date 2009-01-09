import os.path as p, inspect

Import('build')

cwd = p.dirname(inspect.currentframe().f_code.co_filename)
build.kroll_build_dir = p.abspath(p.join(cwd, 'build', build.os))
build.kroll_include_dir = p.abspath(p.join(cwd, 'build', 'include', 'kroll'))

SConscript('api/SConscript', build_dir=build.kroll_build_dir + '/api', duplicate=0)
SConscript('boot/SConscript')
SConscript('host/SConscript')
SConscript('install/SConscript')
SConscript('kernel/SConscript', build_dir=build.kroll_build_dir + '/kernel', duplicate=0)
SConscript('modules/SConscript')
SConscript('test/SConscript', build_dir=build.kroll_build_dir + '/test', duplicate=0)
