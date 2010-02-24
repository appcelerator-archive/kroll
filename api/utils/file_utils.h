/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_FILE_UTILS_H_
#define _KR_FILE_UTILS_H_

#ifdef OS_WIN32
#include <windows.h>
#undef CreateDirectory
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

#ifdef OS_WIN32
#define KR_PATH_SEP_CHAR '\\'
#define KR_PATH_SEP "\\"
#define KR_PATH_SEP_OTHER "/"
#define KR_LIB_SEP ";"
#else
#define KR_PATH_SEP_CHAR '/'
#define KR_PATH_SEP "/"
#define KR_PATH_SEP_OTHER "\\"
#define KR_LIB_SEP ":"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#endif

/**
 * An API for various file utilities (mostly centered around the kroll runtime)
 * Platform-dependent functions can be defined in <os>/<os>_utils.h instead of
 * behind an #ifdef in this file.
 */
namespace UTILS_NS
{
	namespace FileUtils
	{
		/**
		 * Tokenize a string into parts given a string of delimeters characters.
		 */
		KROLL_API void Tokenize(const std::string& haystack,
			std::vector<std::string>& tokens, const std::string& delimeters,
			bool skipDuplicates=false);
		KROLL_API void TokenizeWide(const std::wstring& haystack,
			std::vector<std::wstring>& tokens, const std::wstring& delimeters,
			bool skipDuplicates=false);

		KROLL_API std::string Trim(std::string str);
		KROLL_API void ListDir(const std::string& path, std::vector<std::string>& files);
		KROLL_API bool IsDirectory(const std::string& dir);
		KROLL_API bool IsFile(const std::string& file);
		KROLL_API void WriteFile(const std::string& path, const std::string& content);
		KROLL_API std::string ReadFile(const std::string& path);
		KROLL_API std::string Dirname(const std::string& path);
		KROLL_API std::string Basename(const std::string& path);
		KROLL_API bool CreateDirectory(const std::string& dir, bool recursive=false);
		KROLL_API bool CreateDirectoryImpl(const std::string& dir);

		/**
		 * Recursively delete the directory at the given path
		 */
		KROLL_API bool DeleteDirectory(const std::string& dir);

		/**
		 * This function joins paths together in an OS specific way. Empty elements --
		 * those which equal == "" will be ignored. Examples:
		 * Join('', '/blah', '', 'whatever') => /blah/whatever
		 * Join('', 'blah', 'whatever') => blah/whatever
		 */
		KROLL_API std::string Join(const char*, ...);

		/**
		 * This function returns the Operating system version
		 */
		KROLL_API std::string GetOSVersion();

		/**
		 * This function returns the Operating system architecture
		 */
		KROLL_API std::string GetOSArchitecture();

		/**
		 * This function returns temporary directory for application
		 */
		KROLL_API std::string GetTempDirectory();

		/**
		 * This function returns the path of the currently running executable.
		 * @return the path for the currently running executable
		*/
		KROLL_API std::string GetExecutableDirectory();

		/**
		 * @return the applications data directory
		 */
		KROLL_API std::string GetApplicationDataDirectory(std::string &appid);

		/**
		 * This function indirectly uses the KR_HOME environment variable
		 * set by the boot process
		 * @return the directory for the application's resource files
		*/
		KROLL_API std::string GetResourcesDirectory();

		/**
		 * @return the directory for a given file path
		 */
		KROLL_API std::string GetDirectory(const std::string &file);

		/**
		 * @return true if the given file or directory is hidden.  Otherwise, false is returned.
		 */
		KROLL_API bool IsHidden(const std::string &file);

		/**
		 * Get the system-wide runtime home directory. This is just a
		 * default location --  to get the  current runtime home directory
		 * read the value of the runtime path from the host's current application.
		 */
		KROLL_API std::string GetSystemRuntimeHomeDirectory();

		/**
		 * Get the user-specific runtime home directory. This is just a
		 * default location --  to get the  current runtime home directory
		 * read the value of the runtime path from the host's current application.
		 */
		KROLL_API std::string GetUserRuntimeHomeDirectory();
		KROLL_API bool IsRuntimeInstalled();
		KROLL_API int RunAndWait(std::string& path, std::vector<std::string>& args);
		KROLL_API std::string GetUsername();

#ifndef NO_UNZIP
		typedef bool (*UnzipCallback)(char* message, int current,
			int total, void* data);
		KROLL_API bool Unzip(const std::string& source, const std::string& destination, 
			UnzipCallback callback=0, void* data=0);
#endif
	}
}

#endif
