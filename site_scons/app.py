#!/usr/bin/env python
import os.path as p, os, types, glob, futils, shutil

class App:
	def __init__(self, build, shortname="", fullname="", id="", version="0.1", guid="fakeguid"):
		self.build = build
		self.shortname = shortname
		self.fullname = fullname
		self.id = id
		self.version = version
		self.guid = guid
		self.modules = None # all modules

	def set_modules(self, modules):
		self.modules = modules

	def prestage(self, build_dir, src_contents=None, src_resources=None):
		src_runtime = p.join(self.build.dir, 'runtime')
		src_modules = p.join(self.build.dir, 'modules')
		self.dir = build_dir

		if self.build.is_linux():
			self.contents = build_dir
			self.exe = p.join(self.contents, self.shortname)
			self.kboot = p.join(self.build.dir, 'kboot')
		elif self.build.is_win32():
			self.contents = build_dir
			self.exe = p.join(self.contents, self.shortname+'.exe')
			self.kboot = p.join(self.build.dir, 'kboot.exe')
		elif self.build.is_osx():
			if not self.dir.endswith('.app'): self.dir += '.app'
			self.contents = p.join(self.dir, 'Contents')
			self.exe = p.join(self.contents, 'MacOS', self.shortname)
			self.kboot = p.join(self.build.dir, 'kboot')

			self.runtime = p.join(self.contents, 'runtime');
			self.resources = p.join(self.contents, 'Resources');

			excludes = ['.dll.manifest', '.dll.pdb', '.exp', '.ilk']
			futils.CopyToDir(src_runtime, self.contents, exclude=excludes)
			futils.CopyToDir(src_modules, self.contents, exclude=excludes)
			futils.Copy(self.kboot, self.exe)

			# Copy appinstaller from runtime to contents
			#futils.CopyToDir(src_runtime, self.contents, exclude=excludes)

			if src_contents:
				futils.CopyTree(src_contents, self.contents)
			if src_resources:
				futils.CopyTree(src_resources, self.resources)

	def stage(self, build_dir, src_contents=None, src_resources=None):
		self.prestage(build_dir, src_contents=src_contents, src_resources=src_resources)

		if self.build.is_osx():
			lproj = p.join(self.resources, 'English.lproj')

			# Copy MainMenu.nib to Contents/Resources/English.lproj
			tiui = self.build.get_module('tiui')
			if tiui:
				main_menu = p.join(tiui.build_dir, 'MainMenu.nib')
				futils.CopyToDir(main_menu, lproj) 

			# Copy titanium.icns to Contents/Resources/English.lproj
			icns = p.join(self.build.titanium_support_dir, 'titanium.icns')
			futils.CopyToDir(icns, lproj)

			# Copy Info.plist to Contents
			plist = p.join(self.contents, 'Info.plist')
			futils.CopyToDir(p.join(self.build.titanium_support_dir, 'Info.plist'), self.contents)
			futils.ReplaceVars(plist, {
				'APPEXE': self.shortname,
				'APPNAME': self.fullname,
				'APPICON': 'titanium.icns',
				'APPID': self.id,
				'APPNIB': 'MainMenu',
				'APPVER': self.version})

		# Copy CRT to root
		if self.build.is_win32():
			mscrt = p.join(self.build.third_party, 'microsoft', 'Microsoft.VC80.CRT')
			self.build.utils.CopyToDir(mscrt, self.contents)

		manifest = self.build.generate_manifest(self.fullname, self.id, self.guid)
		m_file = open(p.join(self.contents, 'manifest'), 'w')
		m_file.write(manifest)
		m_file.close()

	def package_dmg(self,
		app_name=None,
		vol_name=None,
		dmg_name=None,
		out_dir=None,
		icns_file=None,
		ds_store_file=None,
		ds_store_extras=[]):

		def invoke(cmd):
			print cmd
			os.system(cmd)

		if not out_dir: out_dir = self.basename(self.dir)
		if not p.isdir(out_dir): os.makedirs(out_dir)

		if not vol_name: vol_name = self.longname
		volume = '/Volumes/%s' % vol_name
		
		if not dmg_name: dmg_name = self.shortname
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
			futils.Copy(icns_file, '%s/.VolumeIcon.icns' % volume)
			invoke('/Developer/Tools/SetFile -a C %s' % volume)
	
		if ds_store_file: futils.Copy(ds_store_file, '%s/.DS_Store' % volume)
		for f in ds_store_extras:
			futils.CopyToDir(f, volume)
			invoke('/Developer/Tools/SetFile -a V %s/%s' % (volume, p.basename(f)))
	
		invoke('hdiutil detach %s' % volume)
		invoke('hdiutil convert "%s" -format UDBZ -imagekey zlib-level=9 -o "%s"' % (temp_dmg, dmg))
		invoke('hdiutil internet-enable -yes "%s"' % dmg)
	
		os.remove(temp_dmg)
