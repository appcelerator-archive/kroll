import SCons.Variables
import SCons.Environment
from SCons.Script import *
import os, os.path as path
import re
import utils

class Module(object):
	script_exts = ['.js', '.rb', '.py']
	def __init__(self, name, version, build_dir, build):
		self.name = name
		self.version = version
		self.build_dir = build_dir
		self.build = build

	def copy_resources(self):
		d = self.build.cwd(2)

		resources = Glob(d + '/AppResources/all') \
		           + Glob(d + '/AppResources/%s' % self.build.os) 
		for r in resources:
			r = path.abspath(str(r))
			self.build.utils.CopyToDir(r, path.join(self.build_dir, 'AppResources'))

		resources = Glob(d + '/Resources/all') \
		           + Glob(d + '/Resources/%s' % self.build.os)
		for r in resources:
			r = path.abspath(str(r))
			self.build.utils.CopyToDir(r, path.join(self.build_dir, 'Resources'))

class BuildUtils(object):
	def __init__(self, env):
		self.env = env
		# Add our custom builders
		env['BUILDERS']['KCopySymlink'] = env.Builder(
			action=utils.KCopySymlink,
			source_factory=SCons.Node.FS.default_fs.Entry,
			target_factory=SCons.Node.FS.default_fs.Entry,
			multi=0)
		env['BUILDERS']['KTarGzDir'] = env.Builder(
			action=utils.KTarGzDir,
			source_factory=SCons.Node.FS.default_fs.Entry,
			target_factory=SCons.Node.FS.default_fs.Entry,
			multi=0)
		env['BUILDERS']['KConcat'] = env.Builder(
			action=utils.KConcat,
			source_factory=SCons.Node.FS.default_fs.Entry,
			target_factory=SCons.Node.FS.default_fs.Entry,
			multi=1)

	def CopyTree(self, *args, **kwargs):
		utils.SCopyTree(self.env, *args, **kwargs)

	def CopyToDir(self, *args, **kwargs):
		utils.SCopyToDir(self.env, *args, **kwargs)

	def Copy(self, src, dest): 
		self.env.Command(dest, src, Copy('$TARGET', '$SOURCE'))

	def Touch(self, file):
		self.env.Command(file, [], Touch('$TARGET'))

	def Delete(self, file):
		self.env.Command(file, [], Delete('$TARGET'))

	def Mkdir(self, file):
		self.env.Command(file, [], Mkdir('$TARGET'))

class BuildConfig(object): 
	def __init__(self, **kwargs):
		self.debug = False
		self.os = None
		self.arch = None # default x86 32-bit
		self.modules = {}
		if not hasattr(os, 'uname') or self.matches('CYGWIN'):
			self.os = 'win32'
		elif self.matches('Darwin'):
			self.os = 'osx'
		elif self.matches('Linux'):
			self.os = 'linux'

			if (os.uname()[4] == 'x86_64'):
				self.arch = '64'

		vars = SCons.Variables.Variables()
		vars.Add('PRODUCT_VERSION', 'The underlying product version for Kroll', kwargs['PRODUCT_VERSION'])
		vars.Add('PRODUCT_NAME', 'The underlying product name that Kroll will display (default: "Kroll")', kwargs['PRODUCT_NAME'])
		vars.Add('INSTALL_PREFIX', 'The install prefix of binaries in the system (default: /usr/local)', kwargs['INSTALL_PREFIX'])
		vars.Add('GLOBAL_NS_VARNAME','The name of the Kroll global variable', kwargs['GLOBAL_NS_VARNAME'])
		vars.Add('CONFIG_FILENAME','The name of the Kroll config file', kwargs['CONFIG_FILENAME'])

		vars.Add('BOOT_RUNTIME_FLAG','The name of the Kroll runtime command line flag', kwargs['BOOT_RUNTIME_FLAG'])
		vars.Add('BOOT_HOME_FLAG','The name of the Kroll home command line file', kwargs['BOOT_HOME_FLAG'])
		vars.Add('BOOT_UPDATESITE_ENVNAME','The name of the Kroll update site environment variable', kwargs['BOOT_UPDATESITE_ENVNAME'])
		vars.Add('BOOT_UPDATESITE_URL','The URL of the Kroll update site', kwargs['BOOT_UPDATESITE_URL'])

		self.env = SCons.Environment.Environment(variables = vars)
		self.utils = BuildUtils(self.env)
		self.env.Append(CPPDEFINES = [
			['OS_' + self.os.upper(), 1],
			['_OS_NAME', self.os],
			['_INSTALL_PREFIX', '${INSTALL_PREFIX}'],
			['_PRODUCT_VERSION', '${PRODUCT_VERSION}'],
			['_PRODUCT_NAME', '${PRODUCT_NAME}'],
			['_GLOBAL_NS_VARNAME', '${GLOBAL_NS_VARNAME}'],
			['_CONFIG_FILENAME' , '${CONFIG_FILENAME}'],
			['_BOOT_RUNTIME_FLAG', '${BOOT_RUNTIME_FLAG}'],
			['_BOOT_HOME_FLAG', '${BOOT_HOME_FLAG}'],
			['_BOOT_UPDATESITE_ENVNAME', '${BOOT_UPDATESITE_ENVNAME}'],
			['_BOOT_UPDATESITE_URL', '${BOOT_UPDATESITE_URL}']
		])

		self.dir = path.abspath(path.join(kwargs['BUILD_DIR'], self.os))
		self.third_party = path.abspath(path.join(kwargs['THIRD_PARTY_DIR'],self.os))
		if (self.arch):
			self.third_party += self.arch

		self.init_thirdparty_libs()


	def init_thirdparty_libs(self):
		self.thirdparty_libs = {
			'poco': {
				'win32': {
					'cpp_path': [path.join(self.third_party, 'poco', 'include')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL', 'PocoUtil', 'PocoXML']	
				},
				'linux': {
					'cpp_path': [path.join(self.third_party, 'poco', 'include')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL', 'PocoUtil', 'PocoXML']	
				},
				'osx': {
					'cpp_path': [path.join(self.third_party, 'poco', 'headers')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL', 'PocoUtil', 'PocoXML']	
				}
			}
		}
	
	def matches(self, n): return bool(re.match(os.uname()[0], n))
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'

	def add_module(self, name, version):
		build_dir = path.join(self.dir, 'modules', name)
		m = Module(name, version, build_dir, self)
		self.modules[name] = m
		return m

	def add_thirdparty(self, env, name, force_libs=False):
		env.Append(CPPPATH=[self.thirdparty_libs[name][self.os]['cpp_path']])
		#if force_libs or not self.is_linux():
		env.Append(LIBPATH=[self.thirdparty_libs[name][self.os]['lib_path']])
		env.Append(LIBS=[self.thirdparty_libs[name][self.os]['libs']])

	def cwd(self, depth=1):
		return path.dirname(sys._getframe(depth).f_code.co_filename)
