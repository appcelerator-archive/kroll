/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <dlfcn.h>
#import "boot.h"

@interface KrollApplicationDelegate : NSObject
-(BOOL)application:(NSApplication*)theApplication
	openFile:(NSString*)filename;
-(BOOL)application:(NSApplication*)theApplication
	openFiles:(NSArray*)filenames;
@end

@implementation KrollApplicationDelegate
-(BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
	return YES;
}

-(BOOL)application:(NSApplication*)theApplication openFiles:(NSArray*)filenames
{
	return YES;
}
@end

namespace KrollBoot
{
	extern string applicationHome;
	extern string updateFile;
	extern SharedApplication app;
	extern int argc;
	extern const char** argv;

	void ShowError(std::string error, bool fatal)
	{
		NSApplicationLoad();

		NSString* buttonText = @"Continue";
		if (fatal)
			buttonText = @"Quit";

		NSRunCriticalAlertPanel(
			[NSString stringWithUTF8String:GetApplicationName().c_str()],
			[NSString stringWithUTF8String:error.c_str()], buttonText, nil, nil);

		if (fatal)
			exit(1);
	}

	string GetApplicationHomePath()
	{
		NSString *bundle = [[NSBundle mainBundle] bundlePath];
		NSString *contents = [NSString stringWithFormat:@"%@/Contents", bundle];
		return std::string([contents UTF8String]);
	}

	void BootstrapPlatformSpecific(string moduleList)
	{
		moduleList = app->runtime->path + ":" + moduleList;

		string path(moduleList);
		string currentFWPath(EnvironmentUtils::Get("DYLD_FRAMEWORK_PATH"));
		EnvironmentUtils::Set("KR_ORIG_DYLD_FRAMEWORK_PATH", currentFWPath);
		if (!currentFWPath.empty())
			path = path + ":" + currentFWPath;

		EnvironmentUtils::Set("DYLD_FRAMEWORK_PATH", path);

		path = moduleList;
		string currentLibPath(EnvironmentUtils::Get("DYLD_LIBRARY_PATH"));
		EnvironmentUtils::Set("KR_ORIG_DYLD_LIBRARY_PATH", currentLibPath);
		if (!currentLibPath.empty())
			path = path + ":" + currentLibPath;

		EnvironmentUtils::Set("DYLD_LIBRARY_PATH", path);

		const char* executablePath = 
			[[[NSBundle mainBundle] executablePath] fileSystemRepresentation];
		EnvironmentUtils::Set("WEBKIT_UNSET_DYLD_FRAMEWORK_PATH", "YES");
		EnvironmentUtils::Set("WebKitAppPath", executablePath);
	}

	string Blastoff()
	{
		// Ensure that the argument list is NULL terminated
		char** myargv = (char **) calloc(sizeof(char *), argc + 1);
		memcpy(myargv, argv, sizeof(char*) * (argc + 1));
		myargv[argc] = 0;
	
		NSString *executablePath = [[NSBundle mainBundle] executablePath];
		execv([executablePath fileSystemRepresentation], myargv);

		// If we get here an error happened with the execv 
		return strerror(errno);
	}

	typedef int Executor(int argc, const char **argv);
	int StartHost()
	{
		// now we need to load the host and get 'er booted
		const char* runtimePath = getenv("KR_RUNTIME");
		if (!runtimePath)
			return __LINE__;

		std::string khost = FileUtils::Join(runtimePath, "libkhost.dylib", 0);
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

	bool RunInstaller(vector<SharedDependency> missing, bool forceInstall)
	{
		string exec = FileUtils::Join(
			app->path.c_str(),
			"installer",
			"Installer App.app",
			"Contents", 
			"MacOS",
			"Installer App", 0);
		if (!FileUtils::IsFile(exec))
		{
			ShowError("Missing installer and application has additional modules that are needed.");
			return false;
		}

		return BootUtils::RunInstaller(missing, app, updateFile);
	}

	string GetApplicationName()
	{
		if (!app.isNull())
		{
			return app->name.c_str();
		}
		else
		{
			// fall back to the info.plist if we haven't loaded the application
			// which happens in a crash situation
			NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
			NSString *applicationName = [infoDictionary objectForKey:@"CFBundleName"];
			if (!applicationName) 
			{
				applicationName = [infoDictionary objectForKey:@"CFBundleExecutable"];
			}
			return [applicationName UTF8String];
		}
	}
}

int main(int argc, char* argv[])
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	KrollBoot::argc = argc;
	KrollBoot::argv = (const char**) argv;
	int rc = 0;

	if (!EnvironmentUtils::Has(BOOTSTRAP_ENV))
	{
		rc = KrollBoot::Bootstrap();
	}
	else
	{
		[[NSApplication sharedApplication] setDelegate:
			[[KrollApplicationDelegate alloc] init]];
			NSApplicationLoad(); EnvironmentUtils::Unset(BOOTSTRAP_ENV);
		rc = KrollBoot::StartHost();
	}

	[pool release];
	return rc;
}
