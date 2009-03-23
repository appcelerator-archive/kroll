import SCons.Variables
import SCons.Environment
from SCons.Script import *
import os, re, utils, types, os.path as path

class Module(object):
	def __init__(self, name, version, build_dir, build):
		self.name = name
		self.version = version
		self.build_dir = build_dir
		self.build = build

	def __str__(self):
		return self.build_dir

	def copy_resources(self, d=None):
		if not d:
			d = self.build.cwd(2)

		resources = Glob(d + '/AppResources/all') \
		           + Glob(d + '/AppResources/%s' % self.build.os) 
		for r in resources:
			r = path.abspath(str(r))
			self.build.utils.CopyToDir(r, path.join(self.build_dir, 'AppResources'))

		resources = Glob(d + '/Resources/all/*') \
		           + Glob(d + '/Resources/%s/*' % self.build.os)
		for r in resources:
			self.build.utils.CopyToDir(path.abspath(str(r)), self.build_dir)

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
			multi=1)
		env['BUILDERS']['KZipDir'] = env.Builder(
			action=utils.KZipDir,
			source_factory=SCons.Node.FS.default_fs.Entry,
			target_factory=SCons.Node.FS.default_fs.Entry,
			multi=1)
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
		return self.env.Command(dest, src, Copy('$TARGET', '$SOURCE'))

	def Touch(self, file):
		return self.env.Command(file, [], Touch('$TARGET'))

	def Delete(self, file):
		return self.env.Command(file, [], Delete('$TARGET'))

	def Mkdir(self, file):
		return self.env.Command(file, [], Mkdir('$TARGET'))

	def ReplaceVars(self, target, replacements):
		self.env.AddPostAction(target, utils.ReplaceVarsAction(target, replacements))

	def WriteStrings(self, target, strings):
		if type(strings) != types.ListType:
			strings = [str(strings)]
		t = self.Touch(target)
		self.env.AddPostAction(t, utils.WriteStringsAction(target, strings))

class BuildConfig(object): 
	def __init__(self, **kwargs):
		self.debug = False
		self.os = None
		self.arch = None # default x86 32-bit
		self.modules = [] 
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
			['_BOOT_UPDATESITE_ENVNAME', '${BOOT_UPDATESITE_ENVNAME}']
		])
		self.version = kwargs['PRODUCT_VERSION']

		self.dir = path.abspath(path.join(kwargs['BUILD_DIR'], self.os))
		self.third_party = path.abspath(path.join(kwargs['THIRD_PARTY_DIR'],self.os))
		if (self.arch):
			self.third_party += self.arch

		self.init_thirdparty_libs()
		self.init_os_arch()


	def init_thirdparty_libs(self):
		self.thirdparty_libs = {
			'poco': {
				'win32': {
					'cpp_path': [path.join(self.third_party, 'poco', 'include')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL', 'PocoUtil', 'PocoXML', 'PocoZip']	
				},
				'linux': {
					'cpp_path': [path.join(self.third_party, 'poco', 'include')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL', 'PocoUtil', 'PocoXML', 'PocoZip']	
				},
				'osx': {
					'cpp_path': [path.join(self.third_party, 'poco', 'headers')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL', 'PocoUtil', 'PocoXML', 'PocoZip']	
				}
			}
		}
	
	def init_os_arch(self):
		if self.is_linux() and self.arch == '64':
			self.env.Append(CPPFLAGS=['-m64', '-Wall', '-Werror','-fno-common','-fvisibility=hidden'])
			self.env.Append(LINKFLAGS=['-m64'])
			self.env.Append(CPPDEFINES = ('OS_64', 1))
			self.x64 = True
			self.x32 = False
		elif self.is_linux() or self.is_osx():
			self.env.Append(CPPFLAGS=['-m32', '-Wall', '-fno-common','-fvisibility=hidden'])
			self.env.Append(LINKFLAGS=['-m32'])
			self.env.Append(CPPDEFINES = ('OS_32', 1))
			self.x64 = False
			self.x32 = True
		else:
			self.env.Append(CPPDEFINES = ('OS_32', 1))
			self.x64 = False
			self.x32 = True

		if self.is_osx():
			if ARGUMENTS.get('osx_10_4', 0):
				OSX_SDK = '/Developer/SDKs/MacOSX10.4u.sdk'
				OSX_MINVERS = '-mmacosx-version-min=10.4'
				self.env['GCC_VERSION'] = '4.0'
				self.env['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
			else:
				OSX_SDK = '/Developer/SDKs/MacOSX10.5.sdk'
				OSX_MINVERS = '-mmacosx-version-min=10.5'

			OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK+' -arch i386 -arch ppc -lstdc++ ' + OSX_MINVERS
			self.env.Append(CXXFLAGS=['-isysroot',OSX_SDK,'-arch','i386',OSX_MINVERS,'-x','objective-c++'])
			self.env.Append(CPPFLAGS=['-arch','i386'])
			self.env.Append(CPPFLAGS=['-arch','ppc'])
			self.env.Append(LINKFLAGS=OSX_UNIV_LINKER)
			self.env.Append(FRAMEWORKS=['Foundation'])
			self.env.Append(CPPFLAGS=['-Wall', '-Werror','-fno-common','-fvisibility=hidden'])

	def matches(self, n): return bool(re.match(os.uname()[0], n))
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'

	def get_module(self, name):
		for module in self.modules:
			if module.name == name:
				return module
		return None

	def add_module(self, name, version=None, resources=True):
		if not version:
			version = self.version

		name = name.lower().replace('.','')
		build_dir = path.join(self.dir, 'modules', name)
		m = Module(name, self.version, build_dir, self)
		self.modules.append(m)

		cwd = self.cwd(2)
		if resources:
			m.copy_resources(cwd)

		return m

	def generate_manifest(self, name, id, guid, excludes=None, includes=None):
		manifest = "#appname: %s\n" % name
		manifest += "#appid: %s\n" % id
		manifest += "#guid: %s\n" % guid
		manifest += "runtime: %s\n" % self.version
		for m in self.modules:
			if (includes and not(m.name in includes)) or \
				(excludes and m.name in excludes):
				continue
			manifest += "%s:%s\n" % (m.name, m.version)
		return manifest

	def add_thirdparty(self, env, name, force_libs=False):
		env.Append(CPPPATH=[self.thirdparty_libs[name][self.os]['cpp_path']])
		env.Append(LIBPATH=[self.thirdparty_libs[name][self.os]['lib_path']])
		env.Append(LIBS=[self.thirdparty_libs[name][self.os]['libs']])

	def cwd(self, depth=1):
		return path.dirname(sys._getframe(depth).f_code.co_filename)
