#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
Import('build')

# things below here will can be shared later in the build
# between both builds -- so basically no Kroll-standalone
# specific stuff

SConscript('api/SConscript', build_dir=build.dir + '/api', duplicate=0)
SConscript('boot/SConscript')
SConscript('host/SConscript')
#SConscript('install/SConscript')
SConscript('kernel/SConscript', build_dir=build.dir + '/kernel', duplicate=0)
SConscript('modules/SConscript')
SConscript('test/SConscript', build_dir=build.dir + '/test')
