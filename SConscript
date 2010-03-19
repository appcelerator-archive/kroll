#!/usr/bin/env python
#         --Common SConscripts--
# Things below here will be shared between Kroll and
# any parent builds (ie Titanium) -- so basically no
# Kroll-specific stuff below here
import os.path as path
Import('build')
Import('debug')

SConscript('boot/SConscript', duplicate=0, build_dir=path.join(build.dir, 'objs', 'boot'))
SConscript('libkroll/SConscript', build_dir=path.join(build.dir,'objs','libkroll'), duplicate=0)

# Now that libkroll is built add it as a default for
# all the following build steps. This means that things
# that should not depend on libkroll should be built
# before here.
build.env.Append(LIBS=['khost'])
build.env.Append(LIBPATH=[build.runtime_build_dir])

SConscript('modules/SConscript')
