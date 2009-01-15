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
#define KR_PATH_SEP "\\"
#define KR_PATH_SEP_OTHER "/"
#ifndef NO_UNZIP
#include "unzip/unzip.h"
#endif
#else
#define KR_PATH_SEP "/"
#define KR_PATH_SEP_OTHER "\\"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#endif

/*
	Constants: Version operators

	EqualTo - =
	GreaterThanEqualTo - >=
	LessThanEqualTo - <=
	GreaterThan - >
	LessThan - <
*/
#define EqualTo 0
#define GreaterThanEqualTo 1
#define LessThanEqualTo 2
#define GreaterThan 3
#define LessThan 4

namespace kroll
{
	/*
	  Class: FileUtils

	  An API for various file utilities (mostly centered around the kroll runtime)
	 */
	class KROLL_API FileUtils
	{
	public:
		/*
		  Function: Trim

		  Trims a string of whitespace

		  Parameters:

		  	str - The string to trim
		 */
		static std::string Trim(std::string str);

		/*
		  Function: ExtractVersion

		  Extracts a matching operation and version from the
			passed-in version spec. For example, a spec of ">= 0.1"
			would set op to <GreaterThanEqualTo> and version to "0.1"

		  Parameters:

		 	spec - a version matching spec, i.e ">= 0.1", "= 0.2", etc
		 	op - will be set to the matching operation:
			<GreaterThanEqualTo>, <LessThanEqualTo>, <LessThan>, <GreaterThan>, <EqualTo>
		 	version - the version from the spec i.e "0.1"
		 */
		static void ExtractVersion(std::string& spec, int *op, std::string &version);

		/*
			Function: MakeVersion

			Turn a version string into an integer representing the version.

			Parameters:

				version - The version string

			Example:

				(start code)
				int version = kroll::FileUtils::MakeVersion("1.2.3");
				// version is now 123
				(end)
		*/
		static int MakeVersion(std::string& version);

		/*
			Function: FindVersioned
		*/
		static std::string FindVersioned(std::string& path, int op, std::string& version);

		/*
			Function: Tokenize
		*/
		static void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters);

		/*
			Function: ReadManifest
		*/
		static bool ReadManifest(std::string& path, std::string &runtimePath, std::vector<std::string>& modules, std::vector<std::string> &moduleDirs);

		/*
			Function: ListDir
		*/
		static void ListDir(std::string& path, std::vector<std::string>& files);

		/*
			Function: IsDirectory
		*/
		static bool IsDirectory(std::string &dir);

		/*
			Function: IsFile
		*/
		static bool IsFile(std::string &file);
		
		/*
		    Function: GetDirectory
		 
		    Returns the directory for a given file path
		 */
		static const char* GetDirectory(std::string &file);

		/*
			Function: IsHidden

			Returns true if the given file or directory is hidden.  Otherwise, false is returned.
		 */
		static bool IsHidden(std::string &file);

		/*
			Function: FindRuntime
		*/
		static std::string FindRuntime(int op, std::string& version);

		/*
			Function: FindModule
		*/
		static std::string FindModule(std::string& name, int op, std::string& version);

		/*
			Function: GetRuntimeBaseDirectory
		*/
		static std::string GetRuntimeBaseDirectory();

		/*
			Function: IsRuntimeInstalled
		*/
		static bool IsRuntimeInstalled();
#ifndef NO_UNZIP
		/*
			Function: Unzip
		*/
		static void Unzip(std::string& source, std::string& destination);
#endif

		/*
			Function: RunAndWait
		*/
		static int RunAndWait(std::string path, std::vector<std::string> args);


	private:
		FileUtils() {}
		~FileUtils() {}
		DISALLOW_EVIL_CONSTRUCTORS(FileUtils);
	};
}

#endif
