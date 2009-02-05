### Win32 specific vars that will need to be setup per-user as environment vars

import os

titanium_includes = []
titanium_libpath = []

if os.environ.has_key("TITANIUM_INCLUDES"):
	titanium_includes.extend(os.environ.get("TITANIUM_INCLUDES").split(";"))

if os.environ.has_key("TITANIUM_LIBPATH"):
	titanium_libpath.extend(os.environ.get("TITANIUM_LIBPATH").split(";"))


build.env.Append(CCFLAGS=['/EHsc', '/MD'])
# disable this for release builds?
build.env.Append(CPPDEFINES={'WIN32_CONSOLE': 1})
build.env.Append(CPPPATH=titanium_includes)
build.env.Append(LIBPATH=titanium_libpath)

build.env.Append(LINKFLAGS=['/DEBUG', '/PDB:${TARGET}.pdb'])
