#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path

filename = inspect.currentframe().f_code.co_filename

Import('product_name')
Import('install_prefix')
Import('global_variable_name')

OS = ''
os_define = ''

vars = Variables()
vars.Add('PRODUCT_NAME', 'The underlying product name that Kroll will display (default: "Kroll")', product_name)
vars.Add('INSTALL_PREFIX', 'The install prefix of binaries in the system (default: /usr/local)', install_prefix)
vars.Add('GLOBAL_NS_VARNAME','The name of the Kroll global variable', global_variable_name)

class BuildConfig(object): 
	def __init__(self):
		if not hasattr(os, 'uname') or self.matches('CYGWIN'):
			self.os = 'win32'
		elif self.matches('Darwin'):
			self.os = 'osx'
		elif self.matches('Linux'):
			self.os = 'linux'
		self.abstopdir = path.abspath('.')
		self.dir = '#build/%s' % self.os 
		self.kroll_build_dir = '#build/%s' % self.os 
		self.absdir = path.abspath('build/%s' % self.os) 
		self.third_party = path.abspath('thirdparty/%s' % self.os)
	def matches(self, n): return bool(re.match(os.uname()[0], n))
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'
	
build = BuildConfig()
build.kroll_include_dir = path.abspath(path.join('build', 'include', 'kroll'))
build.env = Environment(
    variables = vars,
    CPPDEFINES = {
                  'OS_' + build.os.upper(): 1,
                  '_INSTALL_PREFIX': '${INSTALL_PREFIX}',
                  '_PRODUCT_NAME': '${PRODUCT_NAME}',
				  '_GLOBAL_NS_VARNAME': '${GLOBAL_NS_VARNAME}'
                 },
    CPPPATH=[build.abstopdir],
    LIBPATH=[build.dir])

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
    build.env.Append(CPPFLAGS=['-Wall', '-Werror','-fno-common'])

if build.is_osx():
	OSX_SDK = '/Developer/SDKs/MacOSX10.4u.sdk'
	OSX_UNIV_COMPILER = '-isysroot '+OSX_SDK+' -arch i386'
	OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK
	build.env.Append(CXXFLAGS=OSX_UNIV_COMPILER)
	build.env.Append(LDFLAGS=OSX_UNIV_LINKER)


Export ('build')
SConscript('api/SConscript', build_dir=build.dir + '/api', duplicate=0)
SConscript('boot/SConscript')
SConscript('host/SConscript')
SConscript('install/SConscript')
SConscript('kernel/SConscript', build_dir=build.dir + '/kernel', duplicate=0)
SConscript('modules/SConscript')
SConscript('test/SConscript')
