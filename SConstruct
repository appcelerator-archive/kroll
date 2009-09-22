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
	DISTRIBUTION_URL = 'api.appcelerator.net',
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
debug = ARGUMENTS.get('debug', 0)
if debug:
	build.env.Append(CPPDEFINES = ('DEBUG', 1))
	if build.is_win32():
		build.env.Append(CCFLAGS=['/Z7'])  # max debug
		build.env.Append(CPPDEFINES=('WIN32_CONSOLE', 1))
	else:
		build.env.Append(CPPFLAGS=['-g'])  # debug
else:
	build.env.Append(CPPDEFINES = ('NDEBUG', 1 ))
	if not build.is_win32():
		build.env.Append(CPPFLAGS = ['-O9']) # max optimizations

if build.is_win32():
	build.env.Append(CCFLAGS=['/EHsc', '/GR', '/MD'])
	build.env.Append(LINKFLAGS=['/DEBUG', '/PDB:${TARGET}.pdb'])

Export('build')
if 'docs' in COMMAND_LINE_TARGETS:
	SConscript('SConscript.docs')
else:
	SConscript('SConscript', exports='debug')
