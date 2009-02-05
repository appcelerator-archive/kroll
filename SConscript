#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
Import('build')
Import('debug')

# things below here will can be shared later in the build
# between both builds -- so basically no Kroll-standalone
# specific stuff

SConscript('api/SConscript', build_dir=path.join(build.dir,'api'), duplicate=0)
SConscript('boot/SConscript')
SConscript('host/SConscript')
SConscript('modules/SConscript')
SConscript('test/SConscript', build_dir=path.join(build.dir,'test'))
