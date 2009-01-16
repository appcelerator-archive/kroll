/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <api/base.h>
#if defined(OS_OSX)
#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#elif defined(OS_WIN32)
#include <windows.h>
#endif

#include <vector>
#include <string>
#include "testhost.h"
#include <api/module.h>

using namespace kroll;

int main(int argc, const char* argv[])
{
	int rc = 0;
#if defined(OS_OSX)
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
#endif

	const char *basedir = ".";
#if defined(OS_WIN32)
	SetEnvironmentVariableA("KR_HOME", basedir);
	SetEnvironmentVariableA("KR_RUNTIME", basedir);
#else
	setenv("KR_HOME",basedir,1);
	setenv("KR_RUNTIME",basedir,1);
#endif

	std::vector<std::string> modulepaths;
	TestHost *host = new TestHost(modulepaths);
	host->Run();

	std::vector<Module*> modules;
	for (int c=1;c<argc;c++)
	{
		std::string path(argv[c]);
		ModuleProvider *provider = host->FindModuleProvider(path);
		Module *module = provider->CreateModule(path);
		modules.push_back(module);
		module->GetName();
	}

	std::vector<Module*>::iterator i = modules.begin();
	while(i!=modules.end())
	{
		Module* module = (*i++);
		module->Test();
		module->Destroy();
	}

#if defined(OS_OSX)
	[pool release];
#endif

	return rc;
}
