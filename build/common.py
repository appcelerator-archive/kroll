import os, os.path as path
import re
import SCons.Variables
import SCons.Environment

class BuildConfig(object): 
	def __init__(self, **kwargs):
		if not hasattr(os, 'uname') or self.matches('CYGWIN'):
			self.os = 'win32'
		elif self.matches('Darwin'):
			self.os = 'osx'
		elif self.matches('Linux'):
			self.os = 'linux'

		vars = SCons.Variables.Variables()
		vars.Add('PRODUCT_NAME', 'The underlying product name that Kroll will display (default: "Kroll")', kwargs['PRODUCT_NAME'])
		vars.Add('INSTALL_PREFIX', 'The install prefix of binaries in the system (default: /usr/local)', kwargs['INSTALL_PREFIX'])
		vars.Add('GLOBAL_NS_VARNAME','The name of the Kroll global variable', kwargs['GLOBAL_NS_VARNAME'])
		vars.Add('CONFIG_FILENAME','The name of the Kroll config file', kwargs['CONFIG_FILENAME'])
		self.env = SCons.Environment.Environment(variables = vars)
		self.env.Append(CPPDEFINES = {
			'OS_' + self.os.upper(): 1,
			'_INSTALL_PREFIX': '${INSTALL_PREFIX}',
			'_PRODUCT_NAME': '${PRODUCT_NAME}',
			'_GLOBAL_NS_VARNAME': '${GLOBAL_NS_VARNAME}',
			'_CONFIG_FILENAME' : '${CONFIG_FILENAME}'
		})
		self.dir = kwargs['BUILD_DIR'] + '/' + self.os
		self.third_party = kwargs['THIRD_PARTY_DIR'] + '/' + self.os
		self.init_thirdparty_libs()

	def init_thirdparty_libs(self):
		self.thirdparty_libs = {
			'poco': {
				'win32': {
					'cpp_path': [path.join(self.third_party, 'poco', 'include')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL']	
				},
				'linux': {
					'cpp_path': [path.join(self.third_party, 'poco', 'include')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL', 'PocoUtil', 'PocoXML']	
				},
				'osx': {
					'cpp_path': [path.join(self.third_party, 'poco', 'headers')],
					'lib_path': [path.join(self.third_party, 'poco', 'lib')],
					'libs': ['PocoFoundation', 'PocoNet', 'PocoNetSSL']	
				}
			}
		}
	
	def matches(self, n): return bool(re.match(os.uname()[0], n))
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'

	def add_thirdparty(self, env, name):
		print "adding %s lib" % name
		print self.thirdparty_libs[name][self.os]['libs']
		env.Append(LIBPATH=[self.thirdparty_libs[name][self.os]['lib_path']])
		env.Append(CPPPATH=[self.thirdparty_libs[name][self.os]['cpp_path']])
		env.Append(LIBS=[self.thirdparty_libs[name][self.os]['libs']])
