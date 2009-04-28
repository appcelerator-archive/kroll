/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <dlfcn.h>
#import "boot.h"

#ifdef USE_BREAKPAD
#import "client/mac/handler/exception_handler.h"
#import "common/mac/HTTPMultipartUpload.h"
#endif

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
		NSRunCriticalAlertPanel(
			@"Application Error",
			[NSString stringWithCString:error.c_str()], 
			@"Quit", nil, nil);

		if (fatal)
			exit(1);
	}

	string GetApplicationHomePath()
	{
		NSString *bundle = [[NSBundle mainBundle] bundlePath];
		NSString *contents = [NSString stringWithFormat:@"%@/Contents", bundle];
		return std::string([contents UTF8String]);
	}

	bool RunInstaller(vector<SharedDependency> missing)
	{
		string exec = FileUtils::Join(
			app->path.c_str(),
			"installer",
			"Installer App.app",
			"Contents", 
			"MacOS",
			"Installer App", NULL);

		if (!FileUtils::IsFile(exec))
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

		std::vector<SharedDependency>::iterator di = missing.begin();
		while (di != missing.end())
		{
			SharedDependency d = *di++;
			string url = app->GetURLForDependency(d);
			args.push_back(url);
		}

		FileUtils::RunAndWait(exec, args);
		return true;
	}

	void BootstrapPlatformSpecific(string moduleList)
	{
		moduleList = app->runtime->path + ":" + moduleList;

		string path = moduleList;
		string currentFWPath = EnvironmentUtils::Get("DYLD_FRAMEWORK_PATH");
		if (!currentFWPath.empty())
		{
			path = path + ":" + currentFWPath;
		}
		EnvironmentUtils::Set("DYLD_FRAMEWORK_PATH", path);

		path = moduleList;
		string currentLibPath = EnvironmentUtils::Get("DYLD_LIBRARY_PATH");
		if (!currentLibPath.empty())
		{
			path = path + ":" + currentLibPath;
		}
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
		myargv[argc] = NULL;
	
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
		if (runtimePath == NULL)
			return __LINE__;

		std::string khost = FileUtils::Join(runtimePath, "libkhost.dylib", NULL);
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

#ifdef USE_BREAKPAD
	// Allocate this statically because after a crash we want to access
	// the heap as little as possible.
	google_breakpad::ExceptionHandler* breakpad;
	extern string dumpFilePath;

	char breakpadCallBuffer[PATH_MAX];
	bool HandleCrash(
		const char* dumpDirectory, const char* minidumpId, void* context, bool succeeded)
	{
		if (succeeded)
		{
			snprintf(breakpadCallBuffer, PATH_MAX,
				"%s %s %s %s", argv[0], CRASH_REPORT_OPT, dumpDirectory, minidumpId);
			system(breakpadCallBuffer);
		}
		return __LINE__;
	}

	int SendCrashReport()
	{
		if (argc < 3)
		{
			ShowError("Invalid number of arguments passed to crash reporter.");
			return __LINE__;
		}

		NSApplicationLoad();
		NSAlert *alert = [[[NSAlert alloc] init] autorelease];
		[alert setMessageText:@"Titanium has crashed."];
		[alert setInformativeText:
			@"If you send a crash report to Appcelerator, we can try to fix this problem."];
		[alert addButtonWithTitle:@"Send Crash Report"];
		[alert addButtonWithTitle:@"Cancel"];
		int response = [alert runModal];

		if (response == NSAlertFirstButtonReturn)
		{
			map<string, string> params = GetCrashReportParameters();

			NSMutableDictionary* nsParams = [[NSMutableDictionary alloc] init];
			map<string, string>::iterator i = params.begin();
			while (i != params.end())
			{
				NSString* key = [NSString stringWithUTF8String:i->first.c_str()];
				NSString* value = [NSString stringWithUTF8String:i->second.c_str()];
				[nsParams setObject:value forKey:key];
			}

			NSURL* url = [NSURL URLWithString:
				[NSString stringWithUTF8String: STRING(CRASH_REPORT_URL)]];
			HTTPMultipartUpload* uploader = [[HTTPMultipartUpload alloc] initWithURL:url];
			[uploader addFileAtPath:[NSString stringWithUTF8String:dumpFilePath.c_str()] name:@"dump"];
			[uploader setParameters:nsParams];

			NSError* error;
			[uploader send:&error];
			if (error != NULL)
			{
				string msg = "An error occured while sending the crash report: ";
				msg += [[error localizedDescription] UTF8String];
				ShowError(msg);
				return __LINE__;
			}
		}

		return 0;
	}
#endif
}

int main(int argc, char* argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	KrollBoot::argc = argc;
	KrollBoot::argv = (const char**) argv;

#ifdef USE_BREAKPAD
	if (argc > 2 && !strcmp(CRASH_REPORT_OPT, argv[1]))
	{
		KrollBoot::SendCrashReport();
		[pool release];
		return 0;
	}
#endif

	int rc;
	if (!EnvironmentUtils::Has(BOOTSTRAP_ENV))
	{
		rc = KrollBoot::Bootstrap();
	}
	else
	{
#ifdef USE_BREAKPAD
	// Our blastoff execv seems to fail if this handler is installed
	// for the first stage of the boot -- so install it here.
	NSString * tempPath = NSTemporaryDirectory();
	if (tempPath == nil)
		tempPath = @"/tmp";
	string dumpPath = [tempPath UTF8String];
	KrollBoot::breakpad = new google_breakpad::ExceptionHandler(
		dumpPath, NULL, KrollBoot::HandleCrash, NULL, true);
#endif
		EnvironmentUtils::Unset(BOOTSTRAP_ENV);
		rc = KrollBoot::StartHost();
	}

	[pool release];
	return rc;
}
