#!/usr/bin/env python

import os, re, sys, inspect, os.path as path

product_name = 'Kroll'
install_prefix = '/usr/local'
global_variable_name = 'kroll'
config_filename =  'tiapp.xml'

vars = Variables()
vars.Add('PRODUCT_NAME', 'The underlying product name that Kroll will display (default: "Kroll")', product_name)
vars.Add('INSTALL_PREFIX', 'The install prefix of binaries in the system (default: /usr/local)', install_prefix)
vars.Add('GLOBAL_NS_VARNAME','The name of the Kroll global variable', global_variable_name)
vars.Add('CONFIG_FILENAME','The name of the Kroll config file', config_filename)

class BuildConfig(object): 
	def __init__(self):
		if not hasattr(os, 'uname') or self.matches('CYGWIN'):
			self.os = 'win32'
		elif self.matches('Darwin'):
			self.os = 'osx'
		elif self.matches('Linux'):
			self.os = 'linux'
	def matches(self, n): return bool(re.match(os.uname()[0], n))
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'
	def AddPoco(self, env):
		env.Append(LIBPATH=[self.poco_lib])
		env.Append(CPPPATH=[self.poco_inc])
		env.Append(LIBS=['PocoFoundation','PocoNet','PocoNetSSL'])

build = BuildConfig()
build.kroll_source_dir = path.abspath('.')
build.dir = path.abspath('build/' + build.os)
build.third_party = path.abspath('thirdparty/%s' % build.os)
build.kroll_include_dir = path.join(build.dir, 'include')
build.poco = path.join(build.third_party, 'poco')
build.poco_lib = path.join(build.poco, 'lib')
build.poco_inc = path.join(build.poco, 'include')
if build.is_osx():
	build.poco_inc = path.join(build.poco, 'headers')

# This should only be used for accessing various
# scripts in the kroll build directory. All resources
# should instead be built to build.dir
build.kroll_build_dir = path.join(build.kroll_source_dir, 'build')

build.env = Environment(variables=vars)
build.env.Append(CPPDEFINES = {
	'OS_' + build.os.upper(): 1,
	'_INSTALL_PREFIX': '${INSTALL_PREFIX}',
	'_PRODUCT_NAME': '${PRODUCT_NAME}',
	'_GLOBAL_NS_VARNAME': '${GLOBAL_NS_VARNAME}',
	'_CONFIG_FILENAME' : '${CONFIG_FILENAME}'
	})
build.env.Append(CPPPATH=[build.kroll_source_dir])
build.env.Append(LIBPATH=[build.dir])

# debug build flags
if ARGUMENTS.get('debug', 0):
	build.env.Append(CPPDEFINES = {'DEBUG' : 1})
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-g'])  # debug
	else:
		build.env.Append(CCFLAGS = ['/Z7','/GR'])  # max debug, C++ RTTI
else:
	build.env.Append(CPPDEFINES = {'NDEBUG' : 1 })
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-O9']) # max optimizations
	else:
		build.env.Append(CCFLAGS = ['/GR']) # C++ RTTI
		

# turn on special debug printouts for reference counting
if ARGUMENTS.get('debug_refcount', 0):
	build.env.Append(CPPDEFINES = {'DEBUG_REFCOUNT': 1})

if build.is_win32():
	execfile('build/win32.py')

if build.is_linux() or build.is_osx():
    build.env.Append(CPPFLAGS=['-Wall', '-Werror','-fno-common','-fvisibility=hidden'])

if build.is_osx():
	OSX_SDK = '/Developer/SDKs/MacOSX10.4u.sdk'
	OSX_UNIV_COMPILER = '-isysroot '+OSX_SDK+' -arch i386'
	OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK
	build.env.Append(CXXFLAGS=OSX_UNIV_COMPILER)
	build.env.Append(LDFLAGS=OSX_UNIV_LINKER)

Export('build')
if 'docs' in COMMAND_LINE_TARGETS:
	SConscript('SConscript.docs')
else:
	SConscript('SConscript')
