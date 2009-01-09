/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 *
 * a host container interface that end-os environments implement
 */
#ifndef _KR_FILE_UTILS_H_
#define _KR_FILE_UTILS_H_

#ifdef OS_WIN32
# include <windows.h>
#endif

#include "base.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#ifdef OS_WIN32
#define PATH_SEP "\\"
#ifndef NO_UNZIP
#include "unzip/unzip.h"
#endif
#else
#define PATH_SEP "/"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#endif

#define EqualTo 0
#define GreaterThanEqualTo 1
#define LessThanEqualTo 2
#define GreaterThan 3
#define LessThan 4

namespace kroll
{
	/**
	 * File utilities
	 */
	class KROLL_API FileUtils 
	{
	public:
		static std::string Trim(std::string str);
		static void ExtractVersion(std::string& spec, int *op, std::string &version);
		static int MakeVersion(std::string& ver);
		static std::string FindVersioned(std::string& path, int op, std::string& version);
		static void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters);
		static void ReadManifest(std::string& path, std::string &runtimePath, std::vector<std::string>& modules, std::vector<std::string> &moduleDirs);
		static void ListDir(std::string& path, std::vector<std::string>& files);
		static bool IsDirectory(std::string &dir);
		static bool IsFile(std::string &file);
		static std::string FindRuntime(int op, std::string& version);
		static std::string FindModule(std::string& name, int op, std::string& version);
		static std::string GetRuntimeBaseDirectory();
		static bool IsRuntimeInstalled();
#ifndef NO_UNZIP
		static void Unzip(std::string& source, std::string& destination);
#endif
		static int RunAndWait(std::string path, std::vector<std::string> args);
		
		
	private:
		FileUtils() {}
		~FileUtils() {}
		DISALLOW_EVIL_CONSTRUCTORS(FileUtils);
	};
}

#endif
