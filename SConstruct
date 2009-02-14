#!/usr/bin/env python

import os, re, sys, inspect, os.path as path
from build.common import BuildConfig

build = BuildConfig(
    PRODUCT_NAME = 'Kroll',
    INSTALL_PREFIX = '/usr/local',
    GLOBAL_NS_VARNAME = 'kroll',
    CONFIG_FILENAME = 'tiapp.xml',
    BUILD_DIR = path.abspath('build'),
    THIRD_PARTY_DIR = path.abspath('thirdparty'),
	BOOT_RUNTIME_FLAG = '--runtime',
	BOOT_HOME_FLAG = '--start',
	BOOT_UPDATESITE_ENVNAME = 'KR_UPDATESITE',
	BOOT_UPDATESITE_URL = ''
)


build.kroll_source_dir = path.abspath('.')
build.kroll_include_dir = path.join(build.dir, 'include')

# This should only be used for accessing various
# scripts in the kroll build directory. All resources
# should instead be built to build.dir
build.kroll_build_dir = path.join(build.kroll_source_dir, 'build')

build.env.Append(CPPPATH=[build.kroll_source_dir, build.kroll_include_dir])
build.env.Append(LIBPATH=[build.dir])

# debug build flags
if ARGUMENTS.get('debug', 0):
	build.env.Append(CPPDEFINES = ('DEBUG', 1))
	build.debug = True
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-g'])  # debug
	else:
		build.env.Append(CCFLAGS = ['/Z7','/GR'])  # max debug, C++ RTTI
else:
	build.env.Append(CPPDEFINES = ('NDEBUG', 1))
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-O9']) # max optimizations
	else:
		build.env.Append(CCFLAGS = ['/GR']) # C++ RTTI
		

# turn on special debug printouts for reference counting
if ARGUMENTS.get('debug_refcount', 0):
	build.env.Append(CPPDEFINES = ('DEBUG_REFCOUNT', 1))

if build.is_win32():
	execfile('build/win32.py')

if build.is_linux() or build.is_osx():
    build.env.Append(CPPFLAGS=['-Wall', '-Werror','-fno-common','-fvisibility=hidden'])

if build.is_osx():
	OSX_SDK = '/Developer/SDKs/MacOSX10.5.sdk'
	OSX_UNIV_COMPILER = '-isysroot '+OSX_SDK+' -arch i386 -mmacosx-version-min=10.5 -x objective-c++'
	OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK+' -arch i386 -mmacosx-version-min=10.5'
	build.env.Append(CXXFLAGS=OSX_UNIV_COMPILER)
	build.env.Append(LINKFLAGS=OSX_UNIV_LINKER+' -flat_namespace')
	build.env.Append(FRAMEWORKS=['Foundation'])

Export('build')
if 'docs' in COMMAND_LINE_TARGETS:
	SConscript('SConscript.docs')
else:
	SConscript('SConscript')
