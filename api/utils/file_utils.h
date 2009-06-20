/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_FILE_UTILS_H_
#define _KR_FILE_UTILS_H_

#ifdef OS_WIN32
# include <windows.h>
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
#ifndef NO_UNZIP
#include "unzip/unzip.h"
#endif
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

#ifdef USE_NO_EXPORT
#undef KROLL_API
#define KROLL_API
#endif

namespace UTILS_NS
{
	/**
	 * An API for various file utilities (mostly centered around the kroll runtime)
	 */
	class KROLL_API FileUtils
	{
	public:

		/**
		 * tokenize a string by delimeter into parts and place in vector tokens
		 */
		static void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string delimeters, bool skip_if_found=false);

		/**
		 * @param str The string to trim
		 */
		static std::string Trim(std::string str);

		/**
		 *
		 */
		static void ListDir(std::string& path, std::vector<std::string>& files);

		/**
		 *
		 */
		static bool IsDirectory(std::string &dir);

		/**
		 *
		 */
		static bool IsFile(std::string &file);

		/**
		 *
		 */
		static std::string Dirname(std::string path);

		/**
		 *
		 */
		static std::string Basename(std::string path);

		/**
		 *
		 */
		static bool CreateDirectory(std::string &dir, bool recursive=false);

		// TODO - remove this - had to add it to get modules/ti.Database/databases.cpp to link successfully
		static bool CreateDirectory2(std::string &dir);
#if defined(OS_WIN32)
		// TODO: implement this for other platforms
		static void CopyRecursive(std::string &dir, std::string &dest);
#endif
		/**
		 *
		 */
		static bool DeleteDirectory(std::string &dir);

		/**
		 * This function joins paths together in an OS specific way. Empty elements --
		 * those which equal == "" will be ignored. Examples:
		 * Join('', '/blah', '', 'whatever') => /blah/whatever
		 * Join('', 'blah', 'whatever') => blah/whatever
		 */
		static std::string Join(const char*, ...);

		/**
		 * This function returns the Operating system version
		 */
		static std::string GetOSVersion();

		/**
		 * This function returns the Operating system architecture
		 */
		static std::string GetOSArchitecture();

		/**
		 * This function returns temporary directory for application
		 */
		static std::string GetTempDirectory();

		/**
		 * This function returns the path of the currently running executable.
		 * @return the path for the currently running executable
		*/
		static std::string GetExecutableDirectory();

		/**
		 * @return the applications data directory
		 */
		static std::string GetApplicationDataDirectory(std::string &appid);

		/**
		 * This function indirectly uses the KR_HOME environment variable set by the boot process
		 * @return the directory for the applicatin's resource files
		*/
		static std::string GetResourcesDirectory();

		/**
		 * @return the directory for a given file path
		 */
		static std::string GetDirectory(std::string &file);

		/**
		 * @return true if the given file or directory is hidden.  Otherwise, false is returned.
		 */
		static bool IsHidden(std::string &file);

		/**
		 * Get the system-wide runtime home directory. This is just a
		 * default location --  to get the  current runtime home directory
		 * read the value of the runtime path from the host's current application.
		 */
		static std::string GetSystemRuntimeHomeDirectory();

		/**
		 * Get the user-specific runtime home directory. This is just a
		 * default location --  to get the  current runtime home directory
		 * read the value of the runtime path from the host's current application.
		 */
		static std::string GetUserRuntimeHomeDirectory();

		/**
		 *
		 */
		static bool IsRuntimeInstalled();

#ifndef NO_UNZIP
		typedef void (*UnzipCallback)(char *message, int current, int total, void *data);
		
		/**
		 *
		 */
		static void Unzip(std::string& source, std::string& destination, UnzipCallback callback = NULL, void *data = NULL);
#endif

		/**
		 *
		 */
		static int RunAndWait(std::string& path, std::vector<std::string>& args);

		/**
		 *
		 */
		static std::string GetUsername();

		/**
		 * Convert a file URL to an absolute path
		 */
		static std::string FileURLToPath(std::string url);

		/**
		 * Convert an path to a file URL
		 */
		static std::string PathToFileURL(std::string path);

	private:
		FileUtils() {}
		~FileUtils() {}
		DISALLOW_EVIL_CONSTRUCTORS(FileUtils);
	};
}

#endif
