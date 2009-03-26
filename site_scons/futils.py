<<<<<<< HEAD:site_scons/futils.py
import os.path as path, shutil, types, tarfile, zipfile
from SCons.Script import *

def filter_file(file, include=[], exclude=[], filter=None):
	for suffix in include:
		if file.endswith(suffix):
			return True
	if len(include) > 0:
		return False
	for suffix in exclude:
		if file.endswith(suffix):
			return False
	if filter:
		return filter(file)
	return True

# Adapted from: http://www.scons.org/wiki/AccumulateBuilder
def CopyTree(*args, **kwargs):
	if (type(args[1]) == types.ListType):
		for src in args[1]:
			CopyTreeImpl(args[0], src, **kwargs)
	else:
		CopyTreeImpl(*args, **kwargs)

def CopyTreeImpl(src, dest, **kwargs):
	"""Copy a directory recursivley. If the first argument is a
	directory, copy the contents of that directory
	into the target directory. If the first argument is a file,
	copy that file into the target directory. Will preserve symlinks.

	'includes' is a list of file suffixes to include. If len(include) > 1
	all other files will be skipped. 'excludes' is a list of file suffixes
	to exclude. 'filter' is a function which given the full path to a file,
	will exclude if returns False or include if returns True.
	"""
	dest = path.abspath(dest)
	src = path.abspath(src)

	if path.isdir(src):
		for item in os.listdir(src):
			src_item = path.abspath(path.join(src, item))
			#print "copy tree u %s %s" % (src_item, dest)
			CopyToDir(src_item, dest, **kwargs)
	else:
		CopyToDir(src, dest, **kwargs)

def CopyToDir(*args, **kwargs):
	if (type(args[1]) == types.ListType):
		for src in args[1]:
			CopyToDirImpl(args[0], src, **kwargs)
	else:
		CopyToDirImpl(*args, **kwargs)

def CopyToDirImpl(src, dest, include=[], exclude=[], filter=None, recurse=True):
	"""Copy a path into a destination directory in a sconsy way.
	the original path will be a child of the destination directory.

	Includes is a list of file suffixes to include. If len(include) > 1
	all other files will be skipped. Excludes is a list of file suffixes
	to exclude. Filter is a function which given the full path to a file,
	will exclude is returns False or include if returns True.
	"""

	def copy_item(src, dest):
		dest_dir = path.dirname(dest)
		if not path.exists(dest_dir):
			os.makedirs(dest_dir)

		# Test for a symlink first, because a symlink can
		# also return turn for isdir
		if path.islink(src) and filter_file(src, include, exclude, filter):
			CopySymlink(src, dest) 
			return

		# It doesn't really make sense for includes to
		# apply to folders, so we don't use them here
		if path.isdir(src) and filter_file(src, [], exclude, filter):
			copy_items(src, dest)
			return

		elif filter_file(src, [], exclude, filter):
			if path.exists(dest):
				os.remove(dest)
			os.link(src, dest)

	def copy_items(src, dest):
		#print "copy items %s %s" % (src, dest)
		for item in os.listdir(src):
			src_item = os.path.abspath(os.path.join(src, item))
			dest_item = os.path.join(dest, item)
			copy_item(src_item, dest_item)

	src = os.path.abspath(src)
	bname = os.path.basename(src)
	dest = os.path.abspath(dest)

	dest = os.path.join(dest, bname)
	#print "copy %s %s" % (src, dest)
	copy_item(src, dest)

def CopySymlink(link, new_link):
	linkto = os.readlink(link)

	# Remove link before recreating it
	# use os.stat so we can sense broken links
	try:
		os.remove(new_link)
	except:
		pass
	os.symlink(linkto, new_link)

def walk_dir(dir, callback, include=[], exclude=[]):
	files = os.walk(dir)
	for walk in files:
		for file in walk[2]:
			file = os.path.join(walk[0], file)
			if filter_file(file, include, exclude):
				callback(file)

def TarGzDir(source, dest_file, include=[], exclude=[]):
	tar = tarfile.open(dest_file, 'w:gz')
	for dir in source:
		def tarcb(f):
			arcname = f.replace(dir + os.sep, "")
			tar.add(f, arcname)
		walk_dir(dir, tarcb, include, exclude)
	tar.close()

def ZipDir(source, dest_file, include=[], exclude=[]):
	zip = zipfile.ZipFile(dest_file, 'w', zipfile.ZIP_DEFLATED)
	for dir in source:
		def zipcb(f):
			arcname = f.replace(dir + os.sep, "")
			if os.path.islink(f):
				dest = os.readlink(f)
				attr = zipfile.ZipInfo()
				attr.filename = arcname 
				attr.create_system = 3
				attr.external_attr = 2716663808L
				attr.compress_type = zipfile.ZIP_DEFLATED
				zip.writestr(attr, dest)
			else:
				zip.write(f, arcname, zipfile.ZIP_DEFLATED)
		walk_dir(dir, zipcb, include, exclude)
	zip.close()

def Concat(source, dest_file, nofiles=False):
	out = open(dest_file, 'wb')

	for file in source:
		if not(notfiles) and os.path.exists(file):
			inf = open(file, 'rb')
			out.write(inf.read())
			inf.close()
		else:
			out.write(file)

	out.close()

def ReplaceVars(file, replacements):
	txt = open(file).read()
	for k, v in replacements.iteritems():
		txt = txt.replace(k, v)
	out = open(file, 'w')
	out.write(txt)
	out.close()
=======
import os.path as path, shutil, types, tarfile, zipfile
from SCons.Script import *

def filter_file(file, include=[], exclude=[], filter=None):
	for suffix in include:
		if file.endswith(suffix):
			return True
	if len(include) > 0:
		return False
	for suffix in exclude:
		if file.endswith(suffix):
			return False
	if filter:
		return filter(file)
	return True

# Adapted from: http://www.scons.org/wiki/AccumulateBuilder
def CopyTree(*args, **kwargs):
	if (type(args[1]) == types.ListType):
		for src in args[1]:
			CopyTreeImpl(args[0], src, **kwargs)
	else:
		CopyTreeImpl(*args, **kwargs)

def CopyTreeImpl(src, dest, **kwargs):
	"""Copy a directory recursivley. If the first argument is a
	directory, copy the contents of that directory
	into the target directory. If the first argument is a file,
	copy that file into the target directory. Will preserve symlinks.

	'includes' is a list of file suffixes to include. If len(include) > 1
	all other files will be skipped. 'excludes' is a list of file suffixes
	to exclude. 'filter' is a function which given the full path to a file,
	will exclude if returns False or include if returns True.
	"""
	dest = path.abspath(dest)
	src = path.abspath(src)

	if path.isdir(src):
		for item in os.listdir(src):
			src_item = path.abspath(path.join(src, item))
			#print "copy tree u %s %s" % (src_item, dest)
			CopyToDir(src_item, dest, **kwargs)
	else:
		CopyToDir(src, dest, **kwargs)

def CopyToDir(*args, **kwargs):
	if (type(args[1]) == types.ListType):
		for src in args[1]:
			CopyToDirImpl(args[0], src, **kwargs)
	else:
		CopyToDirImpl(*args, **kwargs)

def CopyToDirImpl(src, dest, include=[], exclude=[], filter=None, recurse=True):
	"""Copy a path into a destination directory in a sconsy way.
	the original path will be a child of the destination directory.

	Includes is a list of file suffixes to include. If len(include) > 1
	all other files will be skipped. Excludes is a list of file suffixes
	to exclude. Filter is a function which given the full path to a file,
	will exclude is returns False or include if returns True.
	"""

	def copy_item(src, dest):
		dest_dir = path.dirname(dest)
		if not path.exists(dest_dir):
			os.makedirs(dest_dir)

		# Test for a symlink first, because a symlink can
		# also return turn for isdir
		if path.islink(src) and filter_file(src, include, exclude, filter):
			CopySymlink(src, dest) 
			return

		# It doesn't really make sense for includes to
		# apply to folders, so we don't use them here
		if path.isdir(src) and filter_file(src, [], exclude, filter):
			copy_items(src, dest)
			return

		elif filter_file(src, [], exclude, filter):
			if path.exists(dest):
				os.remove(dest)
			if hasattr(os, 'link'):
				os.link(src, dest)
			else:
				shutil.copy2(src, dest)

	def copy_items(src, dest):
		#print "copy items %s %s" % (src, dest)
		for item in os.listdir(src):
			src_item = os.path.abspath(os.path.join(src, item))
			dest_item = os.path.join(dest, item)
			copy_item(src_item, dest_item)

	src = os.path.abspath(src)
	bname = os.path.basename(src)
	dest = os.path.abspath(dest)

	dest = os.path.join(dest, bname)
	#print "copy %s %s" % (src, dest)
	copy_item(src, dest)

def CopySymlink(link, new_link):
	linkto = os.readlink(link)

	# Remove link before recreating it
	# use os.stat so we can sense broken links
	try:
		os.remove(new_link)
	except:
		pass
	os.symlink(linkto, new_link)

def walk_dir(dir, callback, include=[], exclude=[]):
	files = os.walk(dir)
	for walk in files:
		for file in walk[2]:
			file = os.path.join(walk[0], file)
			if filter_file(file, include, exclude):
				callback(file)

def TarGzDir(source, dest_file, include=[], exclude=[]):
	tar = tarfile.open(dest_file, 'w:gz')
	for dir in source:
		def tarcb(f):
			arcname = f.replace(dir + os.sep, "")
			tar.add(f, arcname)
		walk_dir(dir, tarcb, include, exclude)
	tar.close()

def ZipDir(source, dest_file, include=[], exclude=[]):
	zip = zipfile.ZipFile(dest_file, 'w', zipfile.ZIP_DEFLATED)
	for dir in source:
		def zipcb(f):
			arcname = f.replace(dir + os.sep, "")
			if os.path.islink(f):
				dest = os.readlink(f)
				attr = zipfile.ZipInfo()
				attr.filename = arcname 
				attr.create_system = 3
				attr.external_attr = 2716663808L
				attr.compress_type = zipfile.ZIP_DEFLATED
				zip.writestr(attr, dest)
			else:
				zip.write(f, arcname, zipfile.ZIP_DEFLATED)
		walk_dir(dir, zipcb, include, exclude)
	zip.close()

def Concat(source, dest_file, nofiles=False):
	out = open(dest_file, 'wb')

	for file in source:
		if not(notfiles) and os.path.exists(file):
			inf = open(file, 'rb')
			out.write(inf.read())
			inf.close()
		else:
			out.write(file)

	out.close()

def ReplaceVars(file, replacements):
	txt = open(file).read()
	for k, v in replacements.iteritems():
		txt = txt.replace(k, v)
	out = open(file, 'w')
	out.write(txt)
	out.close()
>>>>>>> 0e4162b7dfa23c3b363b70bf5c5172161e1dd635:site_scons/futils.py
