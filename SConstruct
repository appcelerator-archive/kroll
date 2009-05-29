#!/usr/bin/env python
import os, re, sys, inspect, os.path as path
from kroll import BuildConfig

debug = False
Import('*')

# this will ensure that you're using the right version of scons
EnsureSConsVersion(1,2,0)
# this will ensure that you're using the right version of python
EnsurePythonVersion(2,5)


build = BuildConfig(
	PRODUCT_VERSION = '0.1',
	PRODUCT_NAME = 'Kroll',
	GLOBAL_NS_VARNAME = 'kroll',
	CONFIG_FILENAME = 'tiapp.xml',
	BUILD_DIR = path.abspath('build'),
	THIRD_PARTY_DIR = path.abspath('thirdparty'),
	CRASH_REPORT_URL = 'api.appcelerator.net/p/v1/app-crash-report',
)
build.set_kroll_source_dir(path.abspath('.'))

# This should only be used for accessing various
# scripts in the kroll build directory. All resources
# should instead be built to build.dir
build.kroll_build_dir = path.join(build.kroll_source_dir, 'build')
build.env.Append(CPPPATH=[
	build.kroll_source_dir, 
	build.kroll_include_dir])

# debug build flags
if ARGUMENTS.get('debug', 0) or debug:
	build.env.Append(CPPDEFINES = ('DEBUG', 1))
	build.debug = True
	debug = 1
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-g'])  # debug
	else:
		build.env.Append(CCFLAGS = ['/Z7','/GR'])  # max debug, C++ RTTI
else:
	build.env.Append(CPPDEFINES = ('NDEBUG', 1))
	debug = 0
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-O9']) # max optimizations
	else:
		build.env.Append(CCFLAGS = ['/GR']) # C++ RTTI
		

# turn on special debug printouts for reference counting
if ARGUMENTS.get('debug_refcount', 0):
	build.env.Append(CPPDEFINES = ('DEBUG_REFCOUNT', 1))

if build.is_win32():
	execfile('site_scons/win32.py')


Export('build')

if 'docs' in COMMAND_LINE_TARGETS:
	SConscript('SConscript.docs')
else:
	SConscript('SConscript', exports='debug')
