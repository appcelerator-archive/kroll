#!/usr/bin/env python
#         --Common SConscripts--
# Things below here will be shared between Kroll and
# any parent builds (ie Titanium) -- so basically no
# Kroll-specific stuff below here
import os.path as path
Import('build')
Import('debug')

SConscript('SConscript.thirdparty', duplicate=0)
SConscript('boot/SConscript', duplicate=0, build_dir=path.join(build.dir, 'objs', 'boot'))

SConscript('api/SConscript', build_dir=path.join(build.dir,'objs','api'), duplicate=0)

# Now that libkroll is built add it as a default for
# all the following build steps. This means that things
# that should not depend on libkroll should be built
# before here.
build.env.Append(LIBS=['kroll'])
build.env.Append(LIBPATH=[build.runtime_build_dir])

SConscript('host/SConscript')
SConscript('modules/SConscript')
