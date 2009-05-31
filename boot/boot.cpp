/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "boot.h"

// this is for bundle
#ifdef OS_OSX
#include <Foundation/Foundation.h>
#endif

namespace KrollBoot
{
	string applicationHome;
	string updateFile;
	SharedApplication app = NULL;
	int argc;
	const char** argv;

	void FindUpdate()
	{
		// Search for an update file in the application data  directory.
		// It will be placed there by the update service. If it exists
		// and the version in the update file is greater than the
		// current app version, we want to force an update.
		string file = FileUtils::GetApplicationDataDirectory(app->id);
		file = FileUtils::Join(file.c_str(), UPDATE_FILENAME, NULL);

		std::cerr << ">>>>> UPDATE ===> " << file << std::endl;
				
		if (FileUtils::IsFile(file))
		{
			std::cerr << ">>>>> FOUND UPDATE ===> " << file << std::endl;
			// If we find an update file, we want to resolve the modules that
			// it requires and ignore our current manifest.
			// On error: We should just continue on. A corrupt or old update manifest 
			// doesn't imply that the original application is corrupt.
			SharedApplication update = Application::NewApplication(file, app->path);
			if (!update.isNull())
			{
				std::cerr << ">>>>> FOUND VALID UPDATE ===> " << file << std::endl;
				update->SetArguments(argc, argv);
				app = update;
				updateFile = file;
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
	
		app = Application::NewApplication(applicationHome);
		if (app.isNull())
		{
			ShowError("Application packaging error: could not read manifest.");
			return __LINE__;
		}
		app->SetArguments(argc, argv);

		// Look for a .update file in the app data directory
		FindUpdate();
	
		vector<SharedDependency> missing = app->ResolveDependencies();
		for (size_t i = 0; i < missing.size(); i++)
		{
			SharedDependency d = missing.at(i);
			std::cerr << "Unresolved: " << d->name << " " << d->version << std::endl;
		}

		if (missing.size() > 0 || !app->IsInstalled() || !updateFile.empty())
		{
			if (!RunInstaller(missing))
				return __LINE__;

			missing = app->ResolveDependencies();
		}

		if (missing.size() > 0 || !app->IsInstalled())
		{
			// The user cancelled or the installer encountered an error
			// -- which is should have already reported. We not checking
			// for updateFile.empty() here, because if the user cancelled
			// the update, we just want to start the application as usual.
			return __LINE__;
		}
	
		// Construct a list of module pathnames for setting up library paths
		std::ostringstream moduleList;
		vector<SharedComponent>::iterator i = app->modules.begin();
		while (i != app->modules.end())
		{
			SharedComponent module = *i++;
			moduleList << module->path << MODULE_SEPARATOR;
		}

		EnvironmentUtils::Set(BOOTSTRAP_ENV, "YES");
		EnvironmentUtils::Set("KR_HOME", app->path);
		EnvironmentUtils::Set("KR_RUNTIME", app->runtime->path);
		EnvironmentUtils::Set("KR_MODULES", moduleList.str());

		BootstrapPlatformSpecific(moduleList.str());
		string error = Blastoff();

		// If everything goes correctly, we should never get here
		error = string("Launching application failed: ") + error;
		ShowError(error, false);
		return __LINE__;
	}
	string GetApplicationName()
	{
		if (!app.isNull())
		{
			return app->name.c_str();
		}
#ifdef OS_OSX
		// fall back to the info.plist if we haven't loaded the application
		// which happens in a crash situation
		NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
		NSString *applicationName = [infoDictionary objectForKey:@"CFBundleName"];
		if (!applicationName) 
		{
			applicationName = [infoDictionary objectForKey:@"CFBundleExecutable"];
		}
		return [applicationName UTF8String];
#else
		return PRODUCT_NAME;
#endif
	}

#ifdef USE_BREAKPAD
	string GetCrashDetectionTitle()
	{
		// osx displays title inside the dialog but others are in titlebar
#ifdef OS_OSX
		return app->name + " appears to have encountered a fatal error and cannot continue.";
#else
		return GetApplicationName() + " encountered an error";
#endif		
	}
	string GetCrashDetectionMessage()
	{
		// osx displays title inside the dialog so we can do a partial message
#ifdef OS_OSX
		return "The application has collected information about the error in the form of a detailed error report. If you send the crash report, we will attempt to resolve this problem.";
#else
		return GetApplicationName() + " appears to have encountered a fatal error and cannot continue. The application has collected information about the error in the form of a detailed error report. If you send the crash report, we will attempt to resolve this problem.";
#endif
	}
	void InitCrashDetection()
	{
		// we need to load this since in a crash situation
		// we restart back and by-pass the normal load mechanism
		applicationHome = GetApplicationHomePath();
		string manifestPath = FileUtils::Join(applicationHome.c_str(), MANIFEST_FILENAME, NULL);
		if (FileUtils::IsFile(manifestPath))
		{
			app = Application::NewApplication(applicationHome);
		}
	}
	string dumpFilePath;
	map<string, string> GetCrashReportParameters()
	{
		map<string, string> params;
		if (argc > 3)
		{
			string dumpId = string(argv[3]);
			dumpId +=".dmp";
			dumpFilePath = FileUtils::Join(argv[2], dumpId.c_str(), NULL);

			// send all the stuff that will help us figure out 
			// what the heck is going on and why the shiiiiit is
			// crashing... probably gonna be microsoft's fault
			// at least we can blame it on them...
			if (!app.isNull())
			{
				params["location"] = "desktop"; // this differentiates mobile vs desktop
				params["app_name"] = app->name;
				params["app_id"] = app->id;
				params["app_ver"] = app->version;
				params["app_guid"] = app->guid;
				params["app_home"] = GetApplicationHomePath();
				params["mid"] = PlatformUtils::GetMachineId();
				params["mac"] = PlatformUtils::GetFirstMACAddress();
				params["os"] = OS_NAME;
	#ifdef OS_32
				params["ostype"] = "32bit";
	#elif OS_64
				params["ostype"] = "64bit";
	#else
				params["ostype"] = "unknown";
	#endif
				params["osver"] = FileUtils::GetOSVersion();
				params["osarch"] = FileUtils::GetOSArchitecture();
				params["ver"] = STRING(_PRODUCT_VERSION);
				params["un"] = FileUtils::GetUsername();
				vector<SharedComponent> components;
				app->GetAvailableComponents(components);
				vector<SharedComponent>::const_iterator i = components.begin();
				while(i!=components.end())
				{
					SharedComponent c = (*i++);
					params["module_" + c->name + "_version"] = c->version;
					params["module_" + c->name + "_path"] = c->path;
					params["module_" + c->name + "_bundled"] = c->bundled ? "1":"0";
				}
			}
		}		
		return params;
	}
#endif
}
