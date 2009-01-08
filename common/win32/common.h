/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#define SHARED_INTERFACES
#include "../shared.h"
#undef SHARED_INTERFACES

namespace kroll
{
	std::string getRuntimeBaseDir();
	bool isFile(std::string& str);
	bool isDirectory(std::string& dir);
	bool isRuntimeInstalled();
	std::string getExecutableDirectory();
	void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters);
	int makeVersion(std::string& ver);
	void listDir(std::string& path, std::vector<std::string>& files);
	void readManifest(std::string& path, std::string &runtimePath, std::vector<std::string>& modules, std::vector<std::string> &moduleDirs);
}

#endif
