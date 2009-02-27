import os.path
import shutil
import types
from SCons.Script import *

# Adapted from: http://www.scons.org/wiki/AccumulateBuilder
def SCopyTree(e, src, dest, include=[], exclude=[], filter=None):
	"""Copy a directory recursivley in a sconsy way. If the first
	argument is a directory, copy the contents of that directory
	into the target directory. If the first argument is a file,
	copy that file into the target directory. Will preserve symlinks.
	
	Recursively copy a directory tree using copy2().

	Includes is a list of file suffixes to include. If len(include) > 1
	all other files will be skipped. Excludes is a list of file suffixes
	to exclude. Filter is a function which given the full path to a file,
	will exclude is returns False or include if returns True.
	"""
	def filter_file(file):
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


	def copy_item(src, dest):
		if os.path.isdir(src):
			e.Command(dest, [], Mkdir('$TARGET'))
			copy_items(src, dest)
		elif os.path.islink(src) and filter_file(src):
			e.CopySymlink(dest, src)
		elif filter_file(src):
			e.Command(dest, src, Copy('$TARGET', '$SOURCE'))

	def copy_items(src, dest):
		for item in os.listdir(src):
			src_item = os.path.abspath(os.path.join(src, item))
			dest_item = os.path.join(dest, item)
			copy_item(src_item, dest_item)

	dest = os.path.abspath(dest)
	src = os.path.abspath(src)
	e.Command(dest, [], Mkdir('$TARGET'))

	if os.path.isdir(src):
		copy_items(src, dest)
	else:
		dest_item = os.path.join(dest, os.path.basename(src))
		e.Command(dest_item, src, Copy('$TARGET', '$SOURCE'))

def CopySymlinkBuilder(target, source, env):
	"""Function called when builder is called"""
	link_file = str(source[0])
	dest_file = str(target[0])
	linkto = os.readlink(link_file)
	os.symlink(linkto, dest_file)
