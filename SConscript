#!/usr/bin/env python
#         --Common SConscripts--
# Things below here will be shared between Kroll and
# any parent builds (ie Titanium) -- so basically no
# Kroll-specific stuff below here
import os.path as path
Import('build')
Import('debug')


SConscript('thirdparty/SConscript', duplicate=0)
SConscript('api/SConscript', build_dir=path.join(build.dir,'objs','api'), duplicate=0)
SConscript('boot/SConscript')
SConscript('host/SConscript')
SConscript('modules/SConscript')
SConscript('test/SConscript', build_dir=path.join(build.dir,'objs', 'test'), duplicate=0)
