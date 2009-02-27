#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
Import('build')
Import('debug')

# things below here will be sharedbetween both builds
# -- so basically no Kroll-specific stuff

#SConscript('thirdparty/SConscript', duplicate=0)
SConscript('api/SConscript', build_dir=path.join(build.dir,'api'), duplicate=0)
SConscript('boot/SConscript')
SConscript('host/SConscript')
SConscript('modules/SConscript')
SConscript('test/SConscript', build_dir=path.join(build.dir,'test'))
