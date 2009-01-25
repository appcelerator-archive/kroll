#!/usr/bin/env ruby

KROLL_MODULES = %w(api javascript ruby python)
thisdir = File.expand_path(File.dirname(__FILE__))
outdir = File.join(thisdir, 'build', platform_string)
ext = platform_string=='win32' ? '.exe' : ''
thirdparty = File.join(File.expand_path(File.dirname(__FILE__)+'/thirdparty'), platform_string)
    
task :runtime => [:scons]

def installer?(name)
  if name=~/(kkernel|kroll|khost|kinstall)/
	  return true if platform_string!='win32'
	  return true if platform_string=='win32' and (name.index('.exe') or name.index('.dll'))
  end
  false
end

case platform_string
when 'osx'
	EXT='.dylib'
when 'linux'
	EXT='.so'
when 'win32'
	EXT='.dll'
end

LANGS = {
  '.rb'=>'ruby',
  '.py'=>'python',
  '.js'=>'javascript'
}

task :runtime do
  #FileUtils.rm_rf [installdir,appdir,rootdir]
  FileUtils.mkdir_p [outdir]
  
  runtime_zip = File.join(STAGE_DIR, "kroll-runtime-#{platform_string}-#{KROLL_CONFIG[:version]}.zip")
  FileUtils.rm_rf runtime_zip

  Zip::ZipFile.open(runtime_zip, Zip::ZipFile::CREATE) do |zipfile|
    Dir["#{outdir}/**"].each do |f|
      name = f.gsub(outdir+'/','')
      zipfile.add name,f if installer?(name)
    end
    
    Dir["#{outdir}/*.nib"].each do |f|
      name = f.gsub(outdir+'/','')
      zipfile.add "Resources/English.lproj/#{name}",f
    end

    paths = ['webkit', 'poco', 'poco/lib', 'libcurl']
    paths.each do |path|
      path = "#{thirdparty}/#{path}"
      Dir["#{path}/**/**"].each do |f|
        name = f.gsub(path+'/','')
        zipfile.add name,f unless (name=~/\.h$/ or name=~/\.defs$/)
      end
    end
  end
end

def package_module (mod, outdir, version, zipname="kroll-module-#{mod}-#{version}.zip")
  mod_zip = File.join(STAGE_DIR, zipname)
  FileUtils.rm_rf mod_zip

  Zip::ZipFile.open(mod_zip, Zip::ZipFile::CREATE) do |zipfile|
    Dir["#{outdir}/**"].each do |f|
      name = f.gsub(outdir+'/','')
		  next if File.extname(name)=='.o'
		  lang = LANGS[File.extname(f)]
		  if lang
		    e = File.extname(f)[1..-1]
		    re = "^#{mod}module(.*?)#{e}$"
		    r = Regexp.new re
		    next unless r.match(name)
	    else
  	    next unless name.index(mod) and name.index('module')
	    end
      zipfile.add name,f
    end
		Dir["#{outdir}/modules/*"].each do |f|
			modname = f.gsub(outdir+'/modules/', '')
			modname_nodot = modname.gsub(/\./, '')
			next unless modname_nodot.downcase.index(mod)
			Dir["#{outdir}/modules/#{modname}/Resources/**/*"].each do |r|
				name = r.gsub(outdir+'/modules/'+modname+'/', '')
				zipfile.add name, r
			end
		end
  end
end

task :modules do
  KROLL_MODULES.each do |mod|
    mod.strip!
    package_module(mod, outdir, "0.1")
  end
end