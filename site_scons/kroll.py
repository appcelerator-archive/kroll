import SCons.Variables
import SCons.Environment
from SCons.Script import *
import os, glob, re, utils, futils, types
import os.path as path

class Module(object):
	def __init__(self, name, version, build_dir, build):
		self.name = name
		self.version = version
		self.build_dir = build_dir
		self.build = build

	def __str__(self):
		return self.build_dir

	
	def light_weight_copy(self, name, indir, outdir):
		if os.path.exists(indir):
			t = self.build.env.LightWeightCopyTree(name, [], OUTDIR=outdir, IN=indir, EXCLUDE=['.h'])
			self.build.mark_stage_target(t)
			AlwaysBuild(t)

	def copy_resources(self, d=None):
		if not d:
			d = self.build.cwd(2)

		outdir = self.build_dir
		indir = path.join(d, 'Resources', 'all')
		self.light_weight_copy('#' + self.name + '-AllResources', indir, outdir)

		indir = path.join(d, 'Resources', self.build.os)
		self.light_weight_copy('#' + self.name + '-OSResources', indir, outdir)

		manifest = path.join(d, 'manifest')
		if path.exists(manifest):
			t = self.build.utils.CopyToDir(manifest, self.build_dir)
			self.build.mark_stage_target(t)
			AlwaysBuild(t)

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
		env.SetDefault(TARGZOPTS={})
		env['BUILDERS']['KZipDir'] = env.Builder(
			action=utils.KZipDir,
			source_factory=SCons.Node.FS.default_fs.Entry,
			target_factory=SCons.Node.FS.default_fs.Entry,
			multi=1)
		env.SetDefault(ZIPOPTS={})
		env['BUILDERS']['KConcat'] = env.Builder(
			action=utils.KConcat,
			source_factory=SCons.Node.FS.default_fs.Entry,
			target_factory=SCons.Node.FS.default_fs.Entry,
			multi=1)
		env['BUILDERS']['LightWeightCopyTree'] = env.Builder(action=utils.LightWeightCopyTree)

	def CopyTree(self, *args, **kwargs):
		return utils.SCopyTree(self.env, *args, **kwargs)

	def CopyToDir(self, *args, **kwargs):
		return utils.SCopyToDir(self.env, *args, **kwargs)

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
		return t

	def Zip(self, source, target, **kwargs):
		return self.env.KZipDir(target, source, ZIPOPTS=kwargs)

	def TarGz(self, source, target, **kwargs):
		return self.env.KTarGzDir(target, source, TARGZOPTS=kwargs)

class BuildConfig(object): 
	def __init__(self, **kwargs):
		self.debug = False
		self.os = None
		self.modules = [] 
		if not hasattr(os, 'uname') or self.matches('CYGWIN'):
			self.os = 'win32'
			self.arch = 'i386'

		elif self.matches('Darwin'):
			self.os = 'osx'
			self.arch = 'universal'

		elif self.matches('Linux'):
			self.os = 'linux'
			if (os.uname()[4] == 'x86_64'):
				self.arch = os.uname()[4]
			else:
				self.arch = 'i386'

		vars = SCons.Variables.Variables(args = ARGUMENTS)
		vars.Add('PRODUCT_VERSION', 'The underlying product version for Kroll', kwargs['PRODUCT_VERSION'])
		vars.Add('PRODUCT_NAME', 'The underlying product name that Kroll will display (default: "Kroll")', kwargs['PRODUCT_NAME'])
		vars.Add('GLOBAL_NS_VARNAME','The name of the Kroll global variable', kwargs['GLOBAL_NS_VARNAME'])
		vars.Add('CONFIG_FILENAME','The name of the Kroll config file', kwargs['CONFIG_FILENAME'])
		vars.Add('DISTRIBUTION_URL','The base URL of all streams', kwargs['DISTRIBUTION_URL'])
		vars.Add('CRASH_REPORT_URL','The URL to send crash dumps to', kwargs['CRASH_REPORT_URL'])

		self.env = SCons.Environment.Environment(variables = vars)
		self.utils = BuildUtils(self.env)
		self.env.Append(CPPDEFINES = [
			['OS_' + self.os.upper(), 1],
			['_OS_NAME', self.os],
			['_PRODUCT_VERSION', '${PRODUCT_VERSION}'],
			['_PRODUCT_NAME', '${PRODUCT_NAME}'],
			['_GLOBAL_NS_VARNAME', '${GLOBAL_NS_VARNAME}'],
			['_CONFIG_FILENAME' , '${CONFIG_FILENAME}'],
			['_BOOT_RUNTIME_FLAG', '${BOOT_RUNTIME_FLAG}'],
			['_BOOT_HOME_FLAG', '${BOOT_HOME_FLAG}'],
			['_DISTRIBUTION_URL', '${DISTRIBUTION_URL}'],
			['_CRASH_REPORT_URL', '${CRASH_REPORT_URL}'],
		])
		self.version = self.env['PRODUCT_VERSION']

		self.dir = path.abspath(path.join(kwargs['BUILD_DIR'], self.os))
		self.dist_dir = path.join(self.dir, 'dist')
		self.runtime_build_dir = path.join(self.dir, 'runtime')
		self.runtime_template_dir = path.join(self.runtime_build_dir, 'template')

		self.env.Append(LIBPATH=[self.dir])

		self.init_os_arch()
		self.build_targets = []  # targets needed before packaging & distribution can occur
		self.staging_targets = []  # staging the module and sdk directories
		self.dist_targets = [] # targets that *are* packaging & distribution
		Alias('build', [])
		Alias('stage', [])
		Alias('dist', [])

		# SCons can't read the Visual Studio settings yet so we
		# have to force it to use the Platform SDK directories
		if self.is_win32():
			self.env.Prepend(PATH=['C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2'])
			self.env.Prepend(CPPPATH=['C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\include'])
			self.env.Prepend(LIBPATH=['C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\lib'])
			atlmfc_path = 'C:\\Program Files\\Microsoft Visual Studio 8\\VC\\atlmfc'
			if not path.exists(atlmfc_path):
				atlmfc_path = 'C:\\Program Files (x86)\\Microsoft Visual Studio 8\\VC\\atlmfc'
			self.env.Prepend(CPPPATH=[path.join(atlmfc_path, 'include')])

	def set_kroll_source_dir(self, dir):
		self.kroll_source_dir = path.abspath(dir)
		self.kroll_include_dir = path.join(self.dir, 'include')
		self.kroll_utils_dir = path.join(self.kroll_source_dir, 'api', 'utils');
		self.kroll_support_dir = path.join(self.kroll_source_dir, 'support', self.os)

	# Get a separate copy of the Kroll Utils for a particular build piece
	# Give: A unique directory for that build piece where the utils should be copied
	def get_kroll_utils(self, dir, unzip=True):
		futils.CopyToDir(self.kroll_utils_dir, dir)
		sources = Glob('%s/utils/*.cpp' % dir) + \
			Glob('%s/utils/poco/*.cpp' % dir) + \
			Glob('%s/utils/%s/*.cpp' % (dir, self.os))
		if self.is_win32() and unzip:
			sources.extend(Glob('%s/utils/unzip/*.cpp' % dir))
		if self.is_osx():
			sources.extend(Glob('%s/utils/%s/*.mm' % (dir, self.os)))
		if self.is_osx() or self.is_linux():
			sources.extend(Glob('%s/utils/posix/*.cpp' % dir))
		return sources

	def init_os_arch(self):
		if self.is_linux() and self.is_64():
			self.env.Append(CPPFLAGS=['-m64', '-Wall', '-Werror','-fno-common','-fvisibility=hidden'])
			self.env.Append(LINKFLAGS=['-m64'])
			self.env.Append(CPPDEFINES = ('OS_64', 1))
		elif self.is_linux() or self.is_osx():
			self.env.Append(CPPFLAGS=['-m32', '-Wall', '-Werror', '-fno-common','-fvisibility=hidden'])
			self.env.Append(LINKFLAGS=['-m32'])
			self.env.Append(CPPDEFINES = ('OS_32', 1))
		else:
			self.env.Append(CPPDEFINES = ('OS_32', 1))

		if self.is_osx():
			if ARGUMENTS.get('osx_10_4', 0):
				sdk_dir = '/Developer/SDKs/MacOSX10.4u.sdk'
				sdk_minversion = '-mmacosx-version-min=10.4'
				self.env['GCC_VERSION'] = '4.0'
				self.env['MACOSX_DEPLOYMENT_TARGET'] = '10.4'
			else:
				sdk_dir = '/Developer/SDKs/MacOSX10.5.sdk'
				sdk_minversion = '-mmacosx-version-min=10.5'
				self.env['MACOSX_DEPLOYMENT_TARGET'] = '10.5'

			self.env['CC'] = ['gcc', '-arch', 'i386', '-arch', 'ppc']
			self.env['CXX'] = ['gcc', '-arch', 'i386', '-arch', 'ppc']
			self.env.Append(FRAMEWORKS=['Foundation'])
			self.env.Append(CXXFLAGS=['-isysroot', sdk_dir, sdk_minversion, '-x', 'objective-c++'])
			self.env.Append(LINKFLAGS=['-isysroot', sdk_dir, '-syslibroot,' + sdk_dir, '-lstdc++', sdk_minversion])
			self.env.Append(CPPFLAGS=[
				'-Wall', '-fno-common', '-fvisibility=hidden',
				'-DMACOSX_DEPLOYMENT_TARGET=' + self.env['MACOSX_DEPLOYMENT_TARGET']])

	def matches(self, n): return os.uname()[0].find(n) != -1
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'
	def is_64(self): return self.arch == 'x86_64'
	def is_32(self): return not self.is_64()

	def get_module(self, name):
		for module in self.modules:
			if module.name == name:
				return module
		return None

	def add_module(self, name, version=None, resources=True, env=None):
		if not version:
			version = self.version

		name = name.lower().replace('.','')
		build_dir = path.join(self.dir, 'modules', name)
		m = Module(name, self.version, build_dir, self)
		self.modules.append(m)

		if env:
			env.Append(CPPDEFINES=[('MODULE_NAME', m.name), ('MODULE_VERSION', m.version)])

		m.copy_resources(self.cwd(2))

		return m

	def build_dist_files(self):
		excludes = ['.dll.manifest', '.dll.pdb', '.exp', '.ilk']

		f = path.join(self.dist_dir, 'runtime-%s.zip' % self.version)
		if os.path.exists(f): os.remove(f)

		self.utils.Zip(self.runtime_build_dir, f, exclude=excludes)

		for m in self.modules:
			f = path.join(self.dist_dir, 'module-%s-%s.zip' % (m.name, m.version))
			if os.path.exists(f): os.remove(f)
			self.utils.Zip(m.build_dir, f, exclude=excludes)

	def generate_manifest(self, name, id, guid, exclude=None, include=None, image=None, publisher=None, url=None, version=None, sdk=False):
		manifest = "#appname: %s\n" % name
		manifest += "#appid: %s\n" % id
		manifest += "#guid: %s\n" % guid

		if version: manifest += "#version: %s\n" % version
		if image: manifest += "#image: %s\n" % image
		if publisher: manifest += "#publisher: %s\n" % publisher
		if url: manifest += "#url: %s\n" % url
		if sdk: manifest += "sdk: %s\n" % self.version

		manifest += "runtime: %s\n" % self.version
		for m in self.modules:
			if (include and not (m.name in include)) \
			  or (exclude and (m.name in exclude)):
				continue
			else:
				manifest += "%s:%s\n" % (m.name, m.version)
		return manifest

	def add_thirdparty(self, env, name):
		cpppath = libpath = libs = None
		if name is 'poco':
			cpppath = [self.tp('poco', 'include')]
			libpath = [self.tp('poco', 'lib')]
			libs = ['PocoFoundation', 'PocoNet', 'PocoUtil', 'PocoXML',
				    'PocoZip', 'PocoData', 'PocoSQLite']

		if name is 'curl' and self.is_win32(): # Don't judge us!
			cpppath = [self.tp('webkit', 'include')]
			libpath = [self.tp('webkit', 'lib')]
			libs = ['libcurl_imp']

		elif name is 'curl':
			cpppath = [self.tp('curl', 'include')]
			libpath = [self.tp('curl', 'lib')]
			libs = ['curl']

		if name is 'webkit':
			if self.is_win32() or self.is_linux():
				cpppath = [self.tp('webkit', 'include')]
				libpath = [self.tp('webkit', 'lib')]

			if self.is_linux():
				cpppath.append(self.tp('webkit', 'include', 'glib-2.0'))

			if self.is_win32():
				suffix = ''
				if ARGUMENTS.get('webkit_debug', None):
					suffix = '_debug'
				libs = ['WebKit', 'WebKitGUID', 'JavaScriptCore']
				libs = [x + suffix for x in libs]

			if self.is_linux():
				libs = ['webkittitanium-1.0']

			if self.is_osx():
				env.Append(FRAMEWORKPATH=[self.tp('webkit')])
				env.Append(FRAMEWORKS=['WebKit', 'JavaScriptCore'])

		if cpppath: env.Append(CPPPATH=cpppath)
		if libpath: env.Append(LIBPATH=libpath)
		if libs: env.Append(LIBS=[libs])

	def tp(self, *parts):
		full_path = self.third_party
		for part in parts:
			full_path = path.join(full_path, part)
		return full_path

	def cwd(self, depth=1):
		return path.dirname(sys._getframe(depth).f_code.co_filename)

	def unpack_targets(self, t, f):
		if not t: return
		if type(t) == types.ListType:
			for x in t: self.unpack_targets(x, f)
		else:
			f(t)

	def mark_build_target(self, t):
		self.unpack_targets(t, self.mark_build_target_impl)
	def mark_build_target_impl(self, t):
		Default(t)
		self.build_targets.append(t)
		Alias('build', self.build_targets)

	def mark_stage_target(self, t):
		self.unpack_targets(t, self.mark_stage_target_impl)
	def mark_stage_target_impl(self, t):
		self.staging_targets.append(t)
		Depends(t, 'build')
		Alias('stage', self.staging_targets)

	def mark_dist_target(self, t):
		self.unpack_targets(t, self.mark_dist_target_impl)
	def mark_dist_target_impl(self, t):
		self.dist_targets.append(t)
		Depends(t, 'stage')
		Alias('dist', self.dist_targets)

