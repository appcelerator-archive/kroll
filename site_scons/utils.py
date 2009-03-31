import os.path as path, shutil, types, tarfile, zipfile, futils, stat
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

def walk_dir(dir, callback, include=[], exclude=[]):
	files = os.walk(dir)
	for walk in files:
		for file in walk[2]:
			file = path.join(walk[0], file)
			if filter_file(file, include, exclude):
				callback(file)

def KTarGzDir(target, source, env):
	exclude = include = []
	opts = env['TARGZOPTS']
	if 'exclude' in opts.keys(): exclude = opts['exclude']
	if 'include' in opts.keys(): include = opts['include']

	dest_file = str(target[0])

	if not path.isdir(path.dirname(dest_file)):
		os.makedirs(path.dirname(dest_file))

	tar = tarfile.open(dest_file, 'w:gz')
	for s in source:
		dir = str(s)
		def tarcb(f):
			arcname = f.replace(dir + os.sep, "")
			tar.add(f, arcname)
		walk_dir(dir, tarcb, include, exclude)
	tar.close()

def KZipDir(target, source, env):
	exclude = include = []
	opts = env['ZIPOPTS']
	if 'exclude' in opts.keys(): exclude = opts['exclude']
	if 'include' in opts.keys(): include = opts['include']

	dest_file = str(target[0])

	if not path.isdir(path.dirname(dest_file)):
		os.makedirs(path.dirname(dest_file))

	zip = zipfile.ZipFile(dest_file, 'w', zipfile.ZIP_DEFLATED)
	for s in source:
		dir = str(s)
		def zipcb(f):
			arcname = f.replace(dir + os.sep, "")
			if path.islink(f):
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

def KConcat(target, source, env):
	dest_file = str(target[0])
	out = open(dest_file, 'wb')

	for file in source:
		file = str(file)
		inf = open(str(file), 'rb')
		out.write(inf.read())
		inf.close()

	out.close()

def KReplaceVars(target, replacements):
	txt = open(target).read()
	for k, v in replacements.iteritems():
		txt = txt.replace(k, v)
	out = open(target, 'w')
	out.write(txt)
	out.close()
def KReplaceVarsStr(target, replacements):
    return 'KReplaceVars(%s, %s)' % (target, replacements)
ReplaceVarsAction = SCons.Action.ActionFactory(KReplaceVars, KReplaceVarsStr)

def KWriteStrings(target, strings):
	out = open(target, 'w')
	for part in strings: out.write(part)
	out.close()

def KWriteStringsStr(target, strings):
    return 'KWriteStrings(%s, %s)' % (target, strings)
WriteStringsAction = SCons.Action.ActionFactory(KWriteStrings, KWriteStringsStr)

def NeedsUpdate(source, target, exclude):
	files = os.walk(source)
	for walk in files:
		for file in walk[2]:
			file = path.join(walk[0], file)
			if filter_file(file, [], exclude):

				out_file = target + os.sep + file.replace(source, '')
				if not path.exists(out_file):
					print ' %s does not exist' % out_file
					return True
				else:
					tstamp_o = os.stat(file)[stat.ST_MTIME]
					tstamp_d = os.stat(out_file)[stat.ST_MTIME]
					if tstamp_o > tstamp_d:
						print '%s is out of date' % out_file
						return True
	return False

def LightWeightCopyTreeImpl(source, target, exclude):
	if not path.isdir(source):
		return
	
	if NeedsUpdate(source, target, exclude):
		print "Copying %s ==> %s" % (source, target)
		futils.CopyTree(source, target, exclude=exclude)
	else:
		print "Already up to date: %s ==> %s" % (source, target)

def LightWeightCopyTree(target, source, env):
	if not 'EXCLUDE' in env:
		exclude = []
	else:
		exclude = env['EXCLUDE']
	if type(env['OUTDIR']) == types.ListType:
		env['OUTDIR'] = env['OUTDIR'][0]

	if type(env['IN']) == types.ListType:
		for source in env['IN']:
			LightWeightCopyTreeImpl(source, env['OUTDIR'], exclude)
	else:
		LightWeightCopyTreeImpl(env['IN'], env['OUTDIR'], exclude)


	
