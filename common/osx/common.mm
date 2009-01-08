/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "common.h"
#include "../shared.h"

namespace kroll
{
	std::string findRuntime(int op, std::string& version)
	{
		NSString *runtime = getRuntimeBaseDirectory();
		NSString *path = [NSString stringWithFormat:@"%@/runtime/osx",runtime];
		std::string p([path UTF8String]);
		return findVersioned(p,op,version);
	}

	std::string findModule(std::string name, int op, std::string& version)
	{
		NSString *runtime = getRuntimeBaseDirectory();
		NSString *path = [NSString stringWithFormat:@"%@/modules/%s",runtime,name.c_str()];
		std::string p([path UTF8String]);
		return findVersioned(p,op,version);
	}

	bool isFile(std::string &file)
	{
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:file.c_str()] isDirectory:&isDir];
		return found && !isDir;
	}

	bool isDirectory(std::string &dir)
	{
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:dir.c_str()] isDirectory:&isDir];
		return found && isDir;
	}

	NSString* getRuntimeBaseDirectory()
	{
		// check to see if we already have a local one
		NSString *localDir = [[NSString stringWithFormat:@"~/Library/Application Support/%s",PRODUCT_NAME] stringByExpandingTildeInPath];
		std::string ls = std::string([localDir UTF8String]);
		if (isDirectory(ls))
		{
			return localDir;
		}

		// first check to see if we can install in system directory by checking
		// if we can write to it
		NSString *systemPath = @"/Library/Application Support";
		if ([[NSFileManager defaultManager] isWritableFileAtPath:systemPath])
		{
			return [systemPath stringByAppendingString:@"/"PRODUCT_NAME];
		}
		// if not, we fall back to installing into user directory
		return localDir;
	}

	BOOL isRuntimeInstalled()
	{
		NSString *dir = getRuntimeBaseDirectory();
		std::string ds = std::string([dir UTF8String]);
		return dir && isDirectory(ds);
	}

	int runTaskAndWait(NSString *path, NSArray *args)
	{
		NSLog(@"running task: %@ with %@",path,args);
		NSTask *cmnd=[[NSTask alloc] init];
		[cmnd setLaunchPath:path];
		[cmnd setArguments:args];
		[cmnd launch];
		[cmnd waitUntilExit];
		int status = [cmnd terminationStatus];
		[cmnd release];
		cmnd = nil;
		return status;
	}

	void unzip(NSString *source, NSString *destination)
	{
		//
		// we don't include gzip since we're on OSX
		// we just let the built-in OS handle extraction for us
		//
		runTaskAndWait(@"/usr/bin/ditto",[NSArray arrayWithObjects:@"--noqtn",@"-x",@"-k",@"--rsrc",source,destination,nil]);
	}	
}


