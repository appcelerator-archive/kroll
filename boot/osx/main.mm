/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <iostream>
#import <sstream>
#import <cstring>
#import <cstdio>
#import <cstdlib>
#import <dlfcn.h>
#import "stdlib.h"
#import <utils.h>

using kroll::Application;
using kroll::KComponent;
using std::string;
using std::vector;
using namespace kroll;

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

// these flags are compiled in to allow them
// to be tailed to the embedded environment
//
#ifndef _BOOT_RUNTIME_FLAG
  #define _BOOT_RUNTIME_FLAG --kruntime
#endif

#ifndef _BOOT_HOME_FLAG
  #define _BOOT_HOME_FLAG --khome
#endif

#ifndef _BOOT_UPDATESITE_ENVNAME
  #define _BOOT_UPDATESITE_ENVNAME UPDATESITE
#endif

#ifndef BOOT_RUNTIME_FLAG
  #define BOOT_RUNTIME_FLAG STRING(_BOOT_RUNTIME_FLAG)
#endif

#ifndef BOOT_HOME_FLAG
  #define BOOT_HOME_FLAG STRING(_BOOT_HOME_FLAG)
#endif

string applicationHome;
string updateFile;
Application* app;

inline void ShowError(std::string error, bool fatal=false)
{
	NSApplicationLoad();
	NSRunCriticalAlertPanel(@"Application Error", [NSString stringWithCString:error.c_str()], @"Quit", nil, nil);
	if (fatal)
		exit(1);
}

std::string GetApplicationHomePath()
{
	NSString *bundle = [[NSBundle mainBundle] bundlePath];
	NSString *contents = [NSString stringWithFormat:@"%@/Contents", bundle];
	return std::string([contents UTF8String]);
}

bool RunInstaller(vector<KComponent*> missing)
{
	string exec = kroll::FileUtils::Join(
		app->path.c_str(),
		"installer",
		"Installer App.app",
		"Contents", 
		"MacOS",
		"Installer App", NULL);
	if (!kroll::FileUtils::IsFile(exec))
	{
		ShowError("Missing installer and application has additional modules that are needed.");
		return false;
	}
	vector<string> args;
	args.push_back("-appPath");
	args.push_back(applicationHome);
	if (!updateFile.empty())
	{
		args.push_back("-updateFile");
		args.push_back(updateFile);
	}

	std::vector<KComponent*>::iterator mi = missing.begin();
	while (mi != missing.end())
	{
		KComponent* mod = *mi++;
		std::string path =
			BootUtils::FindBundledModuleZip(mod->name, mod->version, app->path);
		if (path.empty())
		{
			path = mod->GetURL(app);
		}
		args.push_back(path);
	}

	kroll::FileUtils::RunAndWait(exec, args);
	return true;
}

vector<KComponent*> FindModules()
{
	vector<string> runtimeHomes;

	// Search user runtime home first and the the system runtime home
	runtimeHomes.push_back(FileUtils::GetUserRuntimeHomeDirectory());
	runtimeHomes.push_back(FileUtils::GetSystemRuntimeHomeDirectory());

	vector<KComponent*> unresolved = app->ResolveAllComponents(runtimeHomes);
	if (unresolved.size() > 0)
	{
		vector<KComponent*>::iterator dmi = unresolved.begin();
		while (dmi != unresolved.end())
		{
			KComponent* m = *dmi++;
			std::cout << "Unresolved: " << m->name << std::endl;
		}
		std::cout << "---" << std::endl;
	}
	return unresolved;
}

typedef int Executor(int argc, const char **argv);
static void MyExecve(NSString *executable, NSArray *args, NSDictionary *environment)
{
	char **argv = (char **)calloc(sizeof(char *), [args count] + 1);
	char **env = (char **)calloc(sizeof(char *), [environment count] + 1);
	NSEnumerator *e = [args objectEnumerator];
	NSString *s;
	int i = 0;
	while ((s = [e nextObject]))
	{
		argv[i++] = (char *) [s UTF8String];
	}
	e = [environment keyEnumerator];
	i = 0;
	while ((s = [e nextObject]))
	{
		env[i++] = (char *) [[NSString stringWithFormat:@"%@=%@", s, [environment objectForKey:s]] UTF8String];
	}	
#ifdef DEBUG
	std::cout << "before exec: " << [executable fileSystemRepresentation] << std::endl;
#endif
	execve([executable fileSystemRepresentation], argv, env);
}

void FindUpdate()
{
	string file = FileUtils::GetApplicationDataDirectory(app->id);
	file = FileUtils::Join(file.c_str(), UPDATE_FILENAME, NULL);
	if (FileUtils::IsFile(file))
	{
		updateFile = file;
		Application* update = BootUtils::ReadManifestFile(updateFile, applicationHome);
		if (update == NULL)
		{
			// We should probably just continue on. A corrupt manifest doesn't
			// imply that the original application is corrupt.
			ShowError("Application update error: found corrupt update file.");
		}
		else
		{
			// If we find an update file, we want to resolve the modules that
			// requires and ignore our current manifest.
			app = update;
		}
	}
}

int Bootstrap()
{
	applicationHome = GetApplicationHomePath();
	string manifestPath = FileUtils::Join(applicationHome.c_str(), MANIFEST_FILENAME, NULL);
	if (!FileUtils::IsFile(manifestPath))
	{
		ShowError("Application packaging error: no manifest was found.");
		return __LINE__;
	}

	app = BootUtils::ReadManifest(applicationHome);
	if (app == NULL)
	{
		ShowError("Application packaging error: could not read manifest.");
		return __LINE__;
	}

	// Look for a .update file in the app data directory
	FindUpdate();

	vector<KComponent*> missing = FindModules();
	if (missing.size() > 0 || !app->IsInstalled() || !updateFile.empty())
	{
		if (!RunInstaller(missing))
			return __LINE__;

		missing = FindModules();
		FindUpdate();
	}

	if (missing.size() > 0 || !app->IsInstalled() || !updateFile.empty())
	{
		// Don't throw an error here. The installer returned successfully
		// above so this is likely a cancel or an installer error that (hopefully)
		// has already been reported to the user.
		return __LINE__;
	}

	// Construct a list of module pathnames for setting up library paths
	std::ostringstream moduleList;
	vector<KComponent*>::iterator i = app->modules.begin();
	while (i != app->modules.end())
	{
		KComponent* module = *i++;
		moduleList << module->path << ":";
	}

	NSString *executablePath = [[NSBundle mainBundle] executablePath];
	NSMutableDictionary *environment = [NSMutableDictionary 
		dictionaryWithDictionary:[[NSProcessInfo processInfo] environment]]; 

	// Add runtime directory and all module paths to DYLD_FRAMEWORK_PATH,
	// so modules can be shipped with their own frameworks.
	string moduleListStr = moduleList.str();
	NSString* value = [NSString stringWithFormat:@"%s:%s", app->runtime->path.c_str(), moduleListStr.c_str()];
	NSString *currentPath = [environment objectForKey:@"DYLD_FRAMEWORK_PATH"];
	if (currentPath)
	{
		value = [NSString stringWithFormat:@"%@:%@", value, currentPath];
	}
	[environment setObject:value forKey:@"DYLD_FRAMEWORK_PATH"];

	// Add runtime directory to DYLD_LIBRARY_PATH
	value = [NSString stringWithFormat:@"%s:%s", app->runtime->path.c_str(), moduleListStr.c_str()];
	currentPath = [environment objectForKey:@"DYLD_FRAMEWORK_PATH"];
	if (currentPath)
	{
		value = [NSString stringWithFormat:@"%@:%@", value, currentPath];
	}
	[environment setObject:value forKey:@"DYLD_LIBRARY_PATH"];

	[environment setObject:@"YES" forKey:@"KR_FORK"];
	[environment setObject:@"YES" forKey:@"WEBKIT_UNSET_DYLD_FRAMEWORK_PATH"];
	[environment setObject:executablePath forKey:@"WebKitAppPath"];
	value = [NSString stringWithUTF8String:app->path.c_str()];
	[environment setObject:value forKey:@"KR_HOME"];
	value = [NSString stringWithUTF8String:app->runtime->path.c_str()];
	[environment setObject:value forKey:@"KR_RUNTIME"];
	value = [NSString stringWithUTF8String:moduleListStr.c_str()];
	[environment setObject:value forKey:@"KR_MODULES"];
	value = [NSString stringWithUTF8String:app->runtimeHomePath.c_str()];
	[environment setObject:value forKey:@"KR_RUNTIME_HOME"];
	value = [NSString stringWithUTF8String:app->guid.c_str()];
	[environment setObject:value forKey:@"KR_APP_GUID"];
	value = [NSString stringWithUTF8String:app->id.c_str()];
	[environment setObject:value forKey:@"KR_APP_ID"];

	NSArray* arguments = [[NSProcessInfo processInfo] arguments];
	MyExecve(executablePath, arguments, environment);

	// If everything goes correctly, we should never get here
	char *error = strerror(errno);
	NSString *errorMessage = [NSString stringWithFormat:@"Launching application failed with the error '%s' (%d)", error, errno];
	ShowError([errorMessage UTF8String]);
	return __LINE__;
}

int StartHost(int argc, char** argv)
{
	// now we need to load the host and get 'er booted
	string runtimePath = getenv("KR_RUNTIME");
	std::string khost = FileUtils::Join(runtimePath.c_str(), "libkhost.dylib", NULL);

	if (!FileUtils::IsFile(khost))
	{
		string msg = string("Couldn't find required file:") + khost;
		ShowError(msg);
		return __LINE__;
	}

	void* lib = dlopen(khost.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (!lib)
	{
		string msg = string("Couldn't load file:") + khost + ", error: " + dlerror();
		ShowError(msg);
		return __LINE__;
	}

	Executor *executor = (Executor*)dlsym(lib, "Execute");
	if (!executor)
	{
		string msg = string("Invalid entry point for") + khost;
		ShowError(msg);
		return __LINE__;
	}
	return executor(argc, (const char**)argv);
}

int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	int rc;
	char *fork_flag = getenv("KR_FORK");
	if (!fork_flag)
	{
		rc = Bootstrap();
	}
	else
	{
		unsetenv("KR_FORK");
		rc = StartHost(argc, argv);
	}

	#ifdef DEBUG
	std::cout << "return code: " << rc << std::endl;
	#endif

	[pool release];
	return rc;
}
