#!/usr/bin/env python

import os, re, sys, inspect, os.path as path

product_name = 'Kroll'
install_prefix = '/usr/local'
global_variable_name = 'kroll'
config_filename =  'tiapp.xml'

class Build(object): 
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

build = Build()
build_dir = build.absdir

Export('product_name')
Export('install_prefix')
Export('global_variable_name')
Export('config_filename')
Export('build_dir')

if 'docs' in COMMAND_LINE_TARGETS:
	SConscript('SConscript.docs')
else:
	SConscript('SConscript')
