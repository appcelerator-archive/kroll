#!/usr/bin/env python
#
# kroll module builder
#
#
import os, zipfile, fileinput, string, sys, re

class ModuleBuilder(object): 
	manifest = dict()
	files = list()
	dir = str()
	def validate(self,tokens):
		for token in tokens:
			try:
				value = self.manifest[token]
			except:
				print 'Error in manifest. required key "%s" not found' % token
				sys.exit(1)
	
	def file_os_match(self,fn,targetos):
		if re.match('^win32',fn) != None:
			return targetos == 'win32'
		if re.match('^linux',fn) != None:
			return targetos == 'linux'
		if re.match('^osx',fn) != None:
			return targetos == 'osx'
		return True

	def add_dir_to_zip(self,dir,name,osname):
		if os.path.exists(dir):
			for root,dirs,files in os.walk(dir):
				for file in files:
					path = "%s/%s" % (root,file)
					zippath = string.replace(path,dir+'/','')
					if self.file_os_match(zippath,osname):
						zippath = string.replace(zippath,osname+'/','')
						zipname = "%s/%s" % (name,zippath)
						self.add(path,zipname)

	def get_lib_name(self,name,osname):
		if osname=='win32':
			return "%smodule.dll" % name
		if osname=='linux':
			return "lib%smodule.so" % name
		if osname=='osx':
			return "lib%smodule.dylib" % name
		
	def get_module_name(self,name):
		return string.lower(string.replace(name,'.','_'))
		
	def __init__(self,fn,outdir,ostarget):
		dir = os.path.abspath(outdir)
		basedir = os.path.dirname(fn)
		for line in fileinput.input(fn):
			tokens = string.replace(line,'\n','').split(':')
			if len(tokens) == 2:
				self.manifest[tokens[0]]=tokens[1]
		resource_dir = "%s/resources" % basedir
		lib_dir = "%s/libs" % basedir
		self.validate(list(('name','version','description','os')))
		for osname in re.split(',',self.manifest['os']):
			osname = string.replace(osname,' ','')
			if osname == ostarget:
				modulename = self.get_module_name(string.replace(self.manifest['name'],' ',''))
				zipfn = os.path.join(dir,"module_%s_%s.zip" % (modulename,osname))
				self.zip = zipfile.ZipFile(zipfn,'w',zipfile.ZIP_DEFLATED)
				self.zip.comment = 'Kroll Module distribution archive for %s' % self.manifest['name']
				self.add_dir_to_zip(resource_dir,'resources',osname)
				self.add_dir_to_zip(lib_dir,'libs',osname)
				self.add(fn,'manifest')
				libname = self.get_lib_name(modulename,osname)
				lib = os.path.join(outdir,libname)
				self.add(lib,libname)
	
	def add(self,fn,name=''):
		if name == '':
			name = fn
		self.zip.write(fn,name)
		
	def build(self,fn):
		for file in self.files:
			self.zip.write(file)
		self.zip.close()


if len(sys.argv)!=4:
	print "Usage: module.py <outdir> <moduledir> <os>"
	sys.exit(1)

out_dir = sys.argv[1]
module_dir = sys.argv[2]
osname = sys.argv[3]
manifest = os.path.join(module_dir,'manifest')

if os.path.exists(manifest):
	print "building module: %s" % module_dir
	builder = ModuleBuilder(manifest,out_dir,osname)
	builder.build

sys.exit(0)
