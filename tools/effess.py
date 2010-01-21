import os
import os.path as path
import shutil
import types
import tarfile
import zipfile
import stat

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

def copy_tree(*args, **kwargs):
	if (type(args[0]) == types.ListType):
		for src in args[0]:
			copy_tree_impl(src, args[1], **kwargs)
	else:
		copy_tree_impl(*args, **kwargs)

def copy_tree_impl(src, dest, **kwargs):
	"""
	Copy a directory recursivley. If the first argument is a
	directory, copy the contents of that directory
	into the target directory. If the first argument is a file,
	copy that file into the target directory. Will preserve symlinks.
	include - a list of file suffixes to include. If len(include) > 1
	           all other files will be skipped.
	exclude - a list of file suffixes to exclude. 
	filter  - a function which given the full path to a file, returns True
	           to include it or False to exclude it.
	"""
	dest = path.abspath(dest)
	src = path.abspath(src)

	if path.isdir(src):
		for item in os.listdir(src):
			src_item = path.abspath(path.join(src, item))
			#print "copy tree u %s %s" % (src_item, dest)
			copy_to_dir(src_item, dest, **kwargs)
	else:
		copy_to_dir(src, dest, **kwargs)

def copy_to_dir(*args, **kwargs):
	if (type(args[0]) == types.ListType):
		for src in args[0]:
			copy_to_dir_impl(src, args[1], **kwargs)
	else:
		copy_to_dir_impl(*args, **kwargs)

def copy_to_dir_impl(src, dest, include=[], exclude=[], filter=None, recurse=True):
	"""
	Copy a path into a destination directory the original path will be a
	child of the destination directory.
	include - a list of file suffixes to include. If len(include) > 1
	           all other files will be skipped.
	exclude - a list of file suffixes to exclude. 
	filter  - a function which given the full path to a file, returns True
	           to include it or False to exclude it.
	"""

	def copy_item(src, dest):
		dest_dir = path.dirname(dest)
		if not path.exists(dest_dir):
			os.makedirs(dest_dir)

		# Test for a symlink first, because a symlink can
		# also return turn for isdir
		if path.islink(src) and filter_file(src, include, exclude, filter):
			copy_symlink(src, dest) 
			return

		# It doesn't really make sense for includes to
		# apply to folders, so we don't use them here
		if path.isdir(src) and filter_file(src, [], exclude, filter):
			copy_items(src, dest)
			return

		elif filter_file(src, include, exclude, filter):
			if path.exists(dest):
				os.remove(dest)
			shutil.copy2(src, dest)

	def copy_items(src, dest):
		#print "copy items %s %s" % (src, dest)
		if not path.exists(dest):
			os.makedirs(dest)

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

def copy(src, dest):
	dest_dir = path.dirname(dest)
	if not(path.exists(dest_dir)):
		os.makedirs(dest_dir)

	if not path.isdir(src):
		shutil.copy2(src, dest)
	else:
		copy_tree(src, dest)
	

def copy_symlink(link, new_link):
	linkto = os.readlink(link)

	# Remove link before recreating it
	# use os.stat so we can sense broken links
	try:
		os.remove(new_link)
	except:
		pass
	os.symlink(linkto, new_link)

def walk_dir(dir, callback, include=[], exclude=[], dirs=False):
	files = os.walk(dir)
	for walk in files:
		if dirs:
			for dir in walk[1]:
				dir = os.path.join(walk[0], dir)
				if filter_file(dir, [], exclude):
					callback(dir)

		for file in walk[2]:
			file = os.path.join(walk[0], file)
			if filter_file(file, include, exclude):
				callback(file)

def make_tgz(source, dest_file, include=[], exclude=[]):
	tar = tarfile.open(dest_file, 'w:gz')
	if (type(source) != types.ListType): source = [source]
	for dir in source:
		def tarcb(f):
			arcname = f.replace(dir + os.sep, "")
			tar.add(f, arcname)
		walk_dir(dir, tarcb, include, exclude, dirs=True)
	tar.close()

def add_to_zip(zip, filepath, zippath):
	if os.path.islink(filepath):
		dest = os.readlink(filepath)
		attr = zipfile.ZipInfo()
		attr.filename = zippath
		attr.create_system = 3
		attr.external_attr = 2716663808L
		attr.compress_type = zipfile.ZIP_DEFLATED
		zip.writestr(attr, dest)
	elif os.path.isdir(filepath):
		attr = zipfile.ZipInfo(zippath + '/')
		attr.external_attr = 0755 << 16L
		zip.writestr(attr, '')
	else:
		zip.write(filepath, zippath, zipfile.ZIP_DEFLATED)
	
def make_zip(source, dest_file, include=[], exclude=[], dest_path=None):
	zip = None
	if isinstance(dest_file, zipfile.ZipFile):
		zip = dest_file
	else:
		zip = zipfile.ZipFile(dest_file, 'w', zipfile.ZIP_DEFLATED)
	
	if (type(source) != types.ListType): source = [source]
	for dir in source:
		def zipcb(f):
			arcname = f.replace(dir + os.sep, "")
			if dest_path:
				arcname = '/'.join([dest_path, arcname])
			add_to_zip(zip, f, arcname)
		walk_dir(dir, zipcb, include, exclude, dirs=True)
	zip.close()

def concat(source, dest_file, nofiles=False):
	out = open(dest_file, 'wb')

	for file in source:
		if not nofiles and os.path.exists(file):
			inf = open(file, 'rb')
			out.write(inf.read())
			inf.close()
		else:
			out.write(file)

	out.close()

def touch(dest):
	out = open(dest, 'w')
	out.close()

def replace_vars(file, replacements):
	txt = open(file).read()
	for k, v in replacements.iteritems():
		txt = txt.replace(k, v)
	out = open(file, 'w')
	out.write(txt)
	out.close()

def needs_update(source, target, exclude):
	files = os.walk(source)
	for walk in files:
		for file in walk[2]:
			file = path.join(walk[0], file)
			if filter_file(file, [], exclude):

				out_file = target + os.sep + file.replace(source, '')
				if not path.exists(out_file):
					print '    -> %s does not exist' % out_file
					return True
				else:
					tstamp_o = os.stat(file)[stat.ST_MTIME]
					tstamp_d = os.stat(out_file)[stat.ST_MTIME]
					if tstamp_o > tstamp_d:
						print '    -> %s is out of date' % out_file
						return True
	return False

def lightweight_copy_tree_impl(source, target, exclude):
	if not path.isdir(source):
		return
	
	if needs_update(source, target, exclude):
		print "        -> Copying %s ==> %s" % (source, target)
		copy_tree(source, target, exclude=exclude)
	else:
		print "        -> Already up to date: %s"  % target

def lightweight_copy_tree(sources, outdir, exclude=[]):
	if type(sources) == types.ListType:
		for source in sources:
			lightweight_copy_tree_impl(source, outdir, exclude)
	else:
		lightweight_copy_tree_impl(sources, outdir, exclude)

