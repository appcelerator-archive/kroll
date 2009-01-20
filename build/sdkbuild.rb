#!/usr/bin/ruby

require 'fileutils'
require 'zip/zip'

VER=0.1

if RUBY_PLATFORM=~/darwin/
  OS='osx'
elsif RUBY_PLATFORM=~/linux/
  OS='linux'
else
  OS='win32'
end

case OS
when 'osx'
	LIBEXT='.dylib'
	LIBPREFIX='lib'
	EXE=''
when 'linux'
	LIBEXT='.so'
	LIBPREFIX='lib'
	EXE=''
when 'win32'
	LIBEXT='.dll'
	LIBPREFIX=''
	EXE='.exe'
end

executables = %w(kboot kkernel kinstall ktest)
libraries = %w(kroll khost pythonmodule rubymodule javascriptmodule apimodule)

sdk_zip = "kroll-sdk-#{OS}-#{VER}.zip"
FileUtils.rm_rf sdk_zip

Zip::ZipFile.open(sdk_zip, Zip::ZipFile::CREATE) do |zipfile|
	executables.each do |exec|
		zipfile.add 'bin/' + exec + EXE, "#{OS}/#{exec}#{EXE}"
		if OS=='win32'
			zipfile.add "pdb/#{exec}.exe.pdb", "win32/#{exec}.exe.pdb"
		end
	end

	libraries.each do |lib|
		destdir = OS=='win32' ? 'bin' : 'lib'
		libname = "#{LIBPREFIX}#{lib}#{LIBEXT}"
		zipfile.add "#{destdir}/#{libname}", "#{OS}/#{libname}"
		if OS=='win32'
			zipfile.add "lib/#{lib}.lib", "win32/#{lib}.lib"
			zipfile.add "pdb/#{lib}.dll.pdb", "win32/#{lib}.dll.pdb"
		end
	end

	Dir["#{OS}/include/**/*"].each do |f|
		name = f.gsub("#{OS}/include/", '')
		zipfile.add "include/#{name}", f
	end

	Dir["../docs/**/*"].each do |f|
		name = f.gsub("../docs/", '')
		zipfile.add 'docs/'+name, f	
	end
end
