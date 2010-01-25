import shutil
import types
import effess
import os.path as path
from SCons.Script import *

TYPICAL_EXCLUDES = ['.pdb', '.exp', '.ilk', '.db', '.swp', '.swo',
	'.gitignore', '.psd', '.cpp', '.obj', '.pyc', 'SConscript']

class BuildUtils(object):
	def __init__(self, build):
		self.build = build
		self.env = build.env
		# Add our custom builders
		self.env['BUILDERS']['KCopySymlink'] = self.env.Builder(
			action=KCopySymlink,
			source_factory=SCons.Node.FS.default_fs.Entry,
			target_factory=SCons.Node.FS.default_fs.Entry,
			multi=0)
		self.env['BUILDERS']['LightWeightCopyTree'] = self.env.Builder(action=LightWeightCopyTree)

	def CopyTree(self, *args, **kwargs):
		return SCopyTree(self.env, *args, **kwargs)

	def CopyToDir(self, *args, **kwargs):
		return SCopyToDir(self.env, *args, **kwargs)

	def Copy(self, src, dest): 
		return self.env.Command(dest, src, Copy('$TARGET', '$SOURCE'))

	def Touch(self, file):
		return self.env.Command(file, [], Touch('$TARGET'))

	def Delete(self, file):
		return self.env.Command(file, [], Delete('$TARGET'))

	def Mkdir(self, file):
		return self.env.Command(file, [], Mkdir('$TARGET'))

	def LightWeightCopy(self, indir, outdir):
		name = '#'  + (indir + '=' + outdir).replace('/', '-').replace('\\', '-')
		t = self.env.LightWeightCopyTree(name, [], 
			OUTDIR=outdir, IN=indir, EXCLUDE=TYPICAL_EXCLUDES)
		self.build.mark_stage_target(t)
		AlwaysBuild(t)


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
def SCopyTree(*args, **kwargs):
	targets = []
	if (type(args[1]) == types.ListType):
		for src in args[1]:
			t = SCopyTreeImpl(args[0], src, args[2], **kwargs)
			targets.append(t)
		return targets
	else:
		return SCopyTreeImpl(args[0], args[1], args[2], **kwargs)

def SCopyTreeImpl(e, src, dest, **kwargs):
	"""Copy a directory recursivley in a sconsy way. If the first
	argument is a directory, copy the contents of that directory
	into the target directory. If the first argument is a file,
	copy that file into the target directory. Will preserve symlinks.

	Includes is a list of file suffixes to include. If len(include) > 1
	all other files will be skipped. Excludes is a list of file suffixes
	to exclude. Filter is a function which given the full path to a file,
	will exclude is returns False or include if returns True.
	"""
	dest = path.abspath(str(dest))
	src = path.abspath(str(src))

	targets = []
	if path.isdir(src):
		for item in os.listdir(src):
			src_item = path.abspath(path.join(src, item))
			#print "copy tree u %s %s" % (src_item, dest)
			t = SCopyToDir(e, src_item, dest, **kwargs)
			targets.append(t)
		return targets
	else:
		return SCopyToDir(e, src, dest, **kwargs)

def SCopyToDir(*args, **kwargs):
	targets = []
	if (type(args[1]) == types.ListType):
		for src in args[1]:
			t = SCopyToDirImpl(args[0], src, args[2], **kwargs)
			targets.append(t)
		return targets
	else:
		return SCopyToDirImpl(args[0], args[1], args[2], **kwargs)

def SCopyToDirImpl(e, src, dest, include=[], exclude=[], filter=None, recurse=True):
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

		#print "copy u %s %s" % (src, dest)
		# Test for a symlink first, because a symlink can
		# also return turn for isdir
		if path.islink(src) and filter_file(src, include, exclude, filter):
			return e.KCopySymlink(dest, src) 

		# It doesn't really make sense for includes to
		# apply to folders, so we don't use them here
		elif path.isdir(src) and filter_file(src, [], exclude, filter):
			return copy_items(src, dest)

		elif filter_file(src, include, exclude, filter):
			return e.Command(dest, src, Copy('$TARGET', '$SOURCE'))

	def copy_items(src, dest):
		targets = [] # result targets
		if not os.path.exists(dest):
			os.makedirs(dest)
		for item in os.listdir(src):
			src_item = path.abspath(path.join(src, item))
			dest_item = path.join(dest, item)
			t = copy_item(src_item, dest_item)
			targets.append(t)
		return targets

	src = path.abspath(str(src))
	bname = path.basename(src)
	dest = path.abspath(str(dest))

	dest = path.join(dest, bname)
	#print "copy %s %s" % (src, dest)
	return copy_item(src, dest)

def KCopySymlink(target, source, env):
	link_file = str(source[0])
	dest_file = str(target[0])
	linkto = os.readlink(link_file)

	# Remove link before recreating it
	# use os.stat so we can sense broken links
	try:
		os.remove(dest_file)
	except:
		pass
	os.symlink(linkto, dest_file)

def LightWeightCopyTree(target, source, env):
	if not 'EXCLUDE' in env:
		exclude = []
	else:
		exclude = env['EXCLUDE']
	if type(env['OUTDIR']) == types.ListType:
		env['OUTDIR'] = env['OUTDIR'][0]

	effess.lightweight_copy_tree(env['IN'], env['OUTDIR'], exclude)
