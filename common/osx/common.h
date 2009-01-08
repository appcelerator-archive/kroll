/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Foundation/Foundation.h>
#import <api/base.h>

#define SHARED_INTERFACES
#include "../shared.h"
#undef SHARED_INTERFACES

#ifndef _COMMON_H_
#define _COMMON_H_

namespace kroll
{
	NSString* getRuntimeBaseDirectory();
	BOOL isRuntimeInstalled();
	int runTaskAndWait(NSString *path, NSArray *args);
	void unzip(NSString *source, NSString *destination);
	void listDir(std::string& path, std::vector<std::string> &files);
	std::string findRuntime(int op, std::string& version);
	std::string findModule(std::string name, int op, std::string& version);
	bool isFile(std::string &file);
	bool isDirectory(std::string &dir);
}

#endif
