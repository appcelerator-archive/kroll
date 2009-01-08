/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <api/base.h>
#define SHARED_INTERFACES
#include "../shared.h"
#undef SHARED_INTERFACES

#ifndef _INSTALL_PREFIX
	#define _INSTALL_PREFIX "/usr/local"
#endif

#define INSTALL_PREFIX STRING(_INSTALL_PREFIX)

namespace kroll
{
	std::string getRuntimeBaseDir();
	bool isFile(std::string &str);
	bool isDirectory(std::string &dir);
	bool isRuntimeInstalled();
	std::string getExecutableDirectory();
	void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters);
	void readManifest(std::string& path, std::string &runtimePath, std::vector<std::string>& modules, std::vector<std::string> &moduleDirs);
}

#endif
