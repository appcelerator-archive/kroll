#!/usr/bin/env python
import os.path as p
import os
import types
import glob
import shutil
import zipfile
import effess

class App(object):
	def __init__(self, build, fullname="", id="", version="0.1", guid="fakeguid", image=None, publisher=None, url=None, sdk=False):
		self.build = build
		self.fullname = fullname
		self.id = id
		self.version = version
		self.guid = guid
		self.modules = None # all modules
		self.image = image
		self.publisher = publisher
		self.url = url
		self.sdk = sdk

	def installed(self):
		if not p.exists(self.data):
			os.makedirs(self.data)
		f = open(p.join(self.data, '.installed'),'w')
		f.write('#')
		f.close()
		
	def status(self, msg):
		print '    -> %s' % msg

	def set_modules(self, modules):
		self.modules = modules

	def prestage(self, build_dir, src_contents=None, src_resources=None, bundle=True):
		src_runtime = p.join(self.build.dir, 'runtime')
		src_modules = p.join(self.build.dir, 'modules')
		self.dir = build_dir

		if self.build.is_linux():
			self.contents = build_dir
			self.exe = p.join(self.contents, self.fullname)
			self.kboot = p.join(self.build.dir, 'sdk', 'kboot')
			self.data = p.join(p.expanduser('~'), '.titanium', 'appdata', self.id)
		elif self.build.is_win32():
			self.contents = build_dir
			self.exe = p.join(self.contents, self.fullname+'.exe')
			self.kboot = p.join(self.build.dir, 'sdk', 'kboot.exe')
			self.data = p.join(os.environ['APPDATA'], 'Titanium', 'appdata', self.id)
		elif self.build.is_osx():
			if not self.dir.endswith('.app'): self.dir += '.app'
			self.contents = p.join(self.dir, 'Contents')
			self.exe = p.join(self.contents, 'MacOS', self.fullname)
			self.kboot = p.join(self.build.dir, 'sdk', 'kboot')
			self.data = p.join(p.expanduser('~'), 'Library', 'Application Support', 'Titanium', 'appdata', self.id)

		self.runtime = p.join(self.contents, 'runtime');
		self.resources = p.join(self.contents, 'Resources');
		self.net_installer = p.join(self.build.dir, 'sdk', 'installer')

		excludes = ['.pdb', '.exp', '.ilk', '.lib']

		if bundle:
			self.status('copying runtime to %s' % self.contents)
			effess.copy_to_dir(src_runtime, self.contents, exclude=excludes)

			self.status('copying sdk to %s' % self.contents)
			effess.copy_to_dir(p.join(self.build.dir, 'sdk'), self.contents, exclude=excludes)

			self.status('copying modules to %s' % self.contents)
			for module_dir in glob.glob(p.join(src_modules, '*')):
				module_target = p.join(self.contents, 'modules', p.basename(module_dir))
				effess.lightweight_copy_tree(module_dir, module_target, exclude=excludes)

		self.status('copying kboot to %s' % self.exe)
		effess.copy(self.kboot, self.exe)

		self.status('copying netinstaller (%s) to %s' % (self.net_installer, self.contents))
		effess.copy_to_dir(self.net_installer, self.contents)

		if src_contents:
			self.status('copying %s to %s' % (src_contents, self.exe))
			effess.copy_tree(src_contents, self.contents)

		if src_resources:
			self.status('copying %s to %s' % (src_resources, self.exe))
			effess.copy_tree(src_resources, self.resources)

	def stage(self, build_dir, src_contents=None, src_resources=None, bundle=True):
		print('Staging %s' % self.fullname)
		self.prestage(build_dir, src_contents=src_contents, src_resources=src_resources, bundle=bundle)

		if self.build.is_osx():
			self.status('copying mac resources to %s' % (self.contents))
			lproj_dir = p.join(self.resources, 'English.lproj')
			sdk_dir = p.join(self.build.dir, 'sdk')

			# Copy MainMenu.nib & titanium.icns to Contents/Resources/English.lproj
			effess.copy_to_dir(p.join(sdk_dir, 'MainMenu.nib'), lproj_dir)
			effess.copy_to_dir(p.join(sdk_dir, 'titanium.icns'), lproj_dir)

			# Copy Info.plist to Contents
			plist_file = p.join(sdk_dir, 'Info.plist')
			effess.copy_to_dir(plist_file, self.contents)
			effess.replace_vars(plist_file, {
				'APPEXE': self.fullname,
				'APPNAME': self.fullname,
				'APPICON': 'titanium.icns',
				'APPID': self.id,
				'APPNIB': 'MainMenu',
				'APPVER': self.version})
		
		self.status('writing manifest to %s' % p.join(self.contents, 'manifest'))
		manifest = self.build.generate_manifest(
			self.fullname,
			self.id,
			self.guid,
			include=self.modules,
			image=self.image,
			publisher=self.publisher,
			url=self.url,
			version=self.version,
			sdk=self.sdk)
		m_file = open(p.join(self.contents, 'manifest'), 'w')
		m_file.write(manifest)
		m_file.close()

	def package(self, **kwargs):
		print('Packaging %s' % self.fullname)
		if self.build.is_osx():
			self.package_dmg(**kwargs)
		elif self.build.is_linux():
			self.package_tgz(**kwargs)
		elif self.build.is_win32():
			self.package_self_extractor_exe(**kwargs)

	def package_self_extractor_exe(self, out_dir=None, se_archive_name=None, **kwargs):
		if not se_archive_name:
			se_archive_name = p.basename(self.dir)
		if not out_dir: out_dir = p.basename(self.dir)
		if not p.isdir(out_dir): os.makedirs(out_dir)

		exe_out = p.join(p.dirname(self.exe), 'installer.exe')
		effess.copy(self.exe, exe_out)

		self_extractor = p.join(self.build.dir, 'self_extractor.exe')
		se_file = p.join(out_dir, se_archive_name) + '.exe'
		effess.copy(self_extractor, se_file)

		zf = zipfile.ZipFile(se_file, 'a', zipfile.ZIP_DEFLATED)
		for walk in os.walk(self.dir):
			for file in walk[2]:
				file = p.join(walk[0], file)
				arcname = file.replace(self.dir + '\\', "")
				zf.write(file, arcname)
		zf.close()

	def package_tgz(self, out_dir=None, se_archive_name=None, **kwargs):
		if not se_archive_name:
			se_archive_name = p.basename(self.dir)
		if not out_dir: out_dir = p.basename(self.dir)
		if not p.isdir(out_dir): os.makedirs(out_dir)

		archive = p.join(out_dir, se_archive_name) + '.tgz'
		if p.exists(archive): os.remove(archive)

		self.status('writing tgz file to %s' % (archive))
		effess.make_tgz(self.dir, archive)

		#self.status('creating self-extractor at %s' % (bin_file))
		#effess.Concat([source, archive], bin_file)

	def package_dmg(self,
		app_name=None,
		vol_name=None,
		dmg_name=None,
		out_dir=None,
		icns_file=None,
		ds_store_file=None,
		ds_store_extras=[],
		**kwargs):

		def invoke(cmd):
			print cmd
			os.system(cmd)

		if not out_dir: out_dir = self.basename(self.dir)
		if not p.isdir(out_dir): os.makedirs(out_dir)

		if not vol_name: vol_name = self.longname
		volume = '/Volumes/%s' % vol_name
		
		if not dmg_name: dmg_name = self.fullname
		dmg = '%s/%s.dmg' % (out_dir, dmg_name)
		temp_dmg_name = '_' + dmg_name
		temp_dmg = '%s/%s.dmg' % (out_dir, temp_dmg_name)

		if p.exists(dmg): os.remove(dmg)
		if p.exists(temp_dmg): os.remove(temp_dmg)
		if p.exists(temp_dmg_name + '.dmg'): os.remove(temp_dmg_name + '.dmg')
	
		invoke('strip -u -r "%s"' % self.exe) # Strip the binary

		invoke('hdiutil create -srcfolder "%s" -scrub -volname "%s" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "%s"' % (self.dir, vol_name, temp_dmg_name))
		shutil.move(temp_dmg_name + '.dmg', temp_dmg) # hdiutil creates this in the current directory -- move to the build directory

		invoke('hdiutil attach -readwrite -noverify -noautoopen "%s"' % temp_dmg)
		invoke('bless --folder "%s" --openfolder "%s"' % (volume, volume))
	
		if app_name:
			app_name += ".app"
			shutil.move(p.join(volume, p.basename(self.dir)), p.join(volume, app_name))
		else:
			app_name = p.basename(self.dir)

		if icns_file:
			effess.Copy(icns_file, '%s/.VolumeIcon.icns' % volume)
			invoke('/Developer/Tools/SetFile -a C %s' % volume)
	
		if ds_store_file: effess.Copy(ds_store_file, '%s/.DS_Store' % volume)
		for f in ds_store_extras:
			effess.CopyToDir(f, volume)
			invoke('/Developer/Tools/SetFile -a V %s/%s' % (volume, p.basename(f)))
	
		invoke('hdiutil detach %s' % volume)
		invoke('hdiutil convert "%s" -format UDBZ -imagekey zlib-level=9 -o "%s"' % (temp_dmg, dmg))

		# NOTE: removing since this causes auto extraction which we no longer
		# want if we're not using package manager installs
		# invoke('hdiutil internet-enable -yes "%s"' % dmg)
		os.remove(temp_dmg)
