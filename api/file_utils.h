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

#include "base.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#ifdef OS_WIN32
#define KR_PATH_SEP "\\"
#define KR_PATH_SEP_OTHER "/"
#define KR_LIB_SEP ";"
#ifndef NO_UNZIP
#include "unzip/unzip.h"
#endif
#else
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

#define EqualTo 0
#define GreaterThanEqualTo 1
#define LessThanEqualTo 2
#define GreaterThan 3
#define LessThan 4

namespace kroll
{
	/**
	 * An API for various file utilities (mostly centered around the kroll runtime)
	 */
	class KROLL_API FileUtils
	{
	public:
		/**
		 * Extracts a matching operation and version from the
		 * passed-in version spec. For example, a spec of ">= 0.1"
		 * would set op to <GreaterThanEqualTo> and version to "0.1"
		 * @param spec a version matching spec, i.e ">= 0.1", "= 0.2", etc
		 * @param op will be set to the matching operation:
		 * GreaterThanEqualTo, LessThanEqualTo, LessThan, GreaterThan, EqualTo
		 * @param version the version from the spec i.e "0.1"
		 */
		static void ExtractVersion(std::string& spec, int *op, std::string &version);

		/**
		 * Turn a version string into an integer representing the version.
		 * @param version The version string
		 *\code
		 * int version = kroll::FileUtils::MakeVersion("1.2.3");
		 * // version is now 123
		 * \endcode
		*/
		static int MakeVersion(std::string& version);

		/**
		 *
		 */
		static std::string FindVersioned(std::string& path, int op, std::string& version);

		/**
		 *
		 */
		static void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters);

		/**
		 * @param str The string to trim
		 */
		static std::string Trim(std::string str);

		/**
		 *
		 */
		static bool ReadManifest(std::string& path, std::string &runtimePath, std::vector< std::pair< std::pair<std::string,std::string>,bool> >& modules, std::vector<std::string> &moduleDirs, std::string &appname, std::string &appid, std::string &runtimeOverride, std::string &guid);

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
		static bool CreateDirectory(std::string &dir);
#if defined(OS_WIN32)
		// TODO: implement this for other platforms
		static void CopyRecursive(std::string &dir, std::string &dest);
#endif
		/**
		 *
		 */
		static bool DeleteDirectory(std::string &dir);

		/**
		 * This function joins paths together in an OS specific way
		 */
		static std::string Join(const char*, ...);

		/**
		 * This function returns a unique machine identifier
		 */
		static std::string GetMachineId();

		/**
		 * This function returns the Operating system version
		 */
		static std::string GetOSVersion();

		/**
		 * This function returns the Operating system architecture
		 */
		static std::string GetOSArchitecture();

		/**
		 * Encodes a URI value
		 */
		static std::string EncodeURIComponent(std::string value);

		/**
		 * Encodes a URI value
		 */
		static std::string DecodeURIComponent(std::string value);

		/**
		 * This function returns temporary directory for application
		 */
		static std::string GetTempDirectory();

		/**
		 * This function uses the KR_HOME environment variable set by the boot process
		 * @return the directory for the application files
		*/
		static std::string GetApplicationDirectory();

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

		/*
			Function: FindRuntime
		*/
		static std::string FindRuntime(int op, std::string& version);

		/**
		 *
		 */
		static std::string FindModule(std::string& name, int op, std::string& version);

		/**
		 *
		 */
		static std::string GetRuntimeBaseDirectory();

		/**
		 *
		 */
		static bool IsRuntimeInstalled();
#ifndef NO_UNZIP
		/**
		 *
		 */
		static void Unzip(std::string& source, std::string& destination);
#endif

		/**
		 *
		 */
		static int RunAndWait(std::string& path, std::vector<std::string>& args);

		/**
		 *
		 */
		static std::string GetUsername();

	private:
		FileUtils() {}
		~FileUtils() {}
		DISALLOW_EVIL_CONSTRUCTORS(FileUtils);
	};
}

#endif
