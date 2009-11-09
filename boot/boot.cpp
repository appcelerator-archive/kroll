/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "boot.h"

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

		if (FileUtils::IsFile(file))
		{
			// If we find an update file, we want to resolve the modules that
			// it requires and ignore our current manifest.
			// On error: We should just continue on. A corrupt or old update manifest 
			// doesn't imply that the original application is corrupt.
			SharedApplication update = Application::NewApplication(file, app->path);
			if (!update.isNull())
			{
				update->SetArguments(argc, argv);
				app = update;
				updateFile = file;
			}
		}
	}

	vector<SharedDependency> FilterForSDKInstall(
		vector<SharedDependency> dependencies)
	{
		// If this list of dependencies incluces the SDKs, just install
		// those -- we assume that they also supply our other dependencies.
		vector<SharedDependency>::iterator i = dependencies.begin();
		vector<SharedDependency> justSDKs;

		while (i != dependencies.end())
		{
			SharedDependency d = *i++;
			if (d->type == KrollUtils::SDK || d->type == KrollUtils::MOBILESDK)
			{
				justSDKs.push_back(d);
			}
		}

		if (!justSDKs.empty())
		{
			return justSDKs;
		}
		else
		{
			return dependencies;
		}
	}

	int Bootstrap()
	{
		applicationHome = GetApplicationHomePath();
		string manifestPath = FileUtils::Join(applicationHome.c_str(), MANIFEST_FILENAME, NULL);
		if (!FileUtils::IsFile(manifestPath))
		{
			string error("Application packaging error: no manifest was found at: ");
			error.append(manifestPath);
			ShowError(error);
			return __LINE__;
		}

		app = Application::NewApplication(applicationHome);
		if (app.isNull())
		{
			string error("Application packaging error: could not read manifest at: ");
			error.append(manifestPath);
			ShowError(error);
			return __LINE__;
		}
		app->SetArguments(argc, argv);

		// Look for a .update file in the app data directory
		FindUpdate();
	
		vector<SharedDependency> missing = app->ResolveDependencies();
		if (app->HasArgument("debug"))
		{
			vector<SharedComponent> resolved = app->GetResolvedComponents();
			for (size_t i = 0; i < resolved.size(); i++)
			{
				SharedComponent c = resolved[i];
				std::cout << "Resolved: (" << c->name << " " 
					<< c->version << ") " << c->path << std::endl;
			}
			for (size_t i = 0; i < missing.size(); i++)
			{
				SharedDependency d = missing.at(i);
				std::cerr << "Unresolved: " << d->name << " " 
					<< d->version << std::endl;
			}
		}

		bool forceInstall = app->HasArgument("--force-install");
		if (forceInstall || !missing.empty() || !app->IsInstalled() ||
			!updateFile.empty())
		{
			// If this list of dependencies incluces the SDKs, just install
			// those -- we assume that they also supply our other dependencies.
			missing = FilterForSDKInstall(missing);

			if (!RunInstaller(missing, forceInstall))
			{
				return __LINE__;
			}

			missing = app->ResolveDependencies();
		}

		if (missing.size() > 0 || !app->IsInstalled())
		{
			// The user cancelled or the installer encountered an error
			// -- which is should have already reported. We're not checking
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

#ifdef USE_BREAKPAD
	string GetCrashDetectionTitle()
	{
		return GetApplicationName() + " encountered an error";
	}

	string GetCrashDetectionHeader()
	{
		return GetApplicationName() + " appears to have encountered a fatal error and cannot continue.";
	}

	string GetCrashDetectionMessage()
	{
		return "The application has collected information about the error"
		" in the form of a detailed error report. If you send the crash report,"
		" we will attempt to resolve this problem.";
	}

	void InitCrashDetection()
	{
		// Load the application manifest so that we can get lots of debugging
		// information for the crash report.
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
			string dumpId = string(argv[3]) + ".dmp";
			dumpFilePath = FileUtils::Join(argv[2], dumpId.c_str(), NULL);

			// send all the stuff that will help us figure out 
			// what the heck is going on and why the shiiiiiiiiit is
			// crashing... probably gonna be microsoft's fault
			// at least we can blame it on them...

			// this differentiates mobile vs desktop
			params["location"] = "desktop"; 
			params["mid"] = PlatformUtils::GetMachineId();
			params["mac"] = PlatformUtils::GetFirstMACAddress();
			params["os"] = OS_NAME;
			params["ostype"] = OS_TYPE;
			params["osver"] = FileUtils::GetOSVersion();
			params["osarch"] = FileUtils::GetOSArchitecture();
			params["ver"] = PRODUCT_VERSION;
			params["un"] = FileUtils::GetUsername();

			if (!app.isNull())
			{
				params["app_name"] = app->name;
				params["app_id"] = app->id;
				params["app_ver"] = app->version;
				params["app_guid"] = app->guid;
				params["app_home"] = GetApplicationHomePath();

				vector<SharedComponent> components;
				app->GetAvailableComponents(components);
				vector<SharedComponent>::const_iterator i = components.begin();
				while (i != components.end())
				{
					SharedComponent c = (*i++);
					string type("unknown");
					if (c->type == KrollUtils::MODULE)
					{
						type = "module";
					}
					else if (c->type == KrollUtils::RUNTIME)
					{
						type = "runtime";
					}
					else if (c->type == KrollUtils::SDK)
					{
						type = "sdk";
					}
					else if (c->type == KrollUtils::MOBILESDK)
					{
						type = "mobilesdk";
					}
					params[type + "_" + c->name + "_version"] = c->version;
					params[type + "_" + c->name + "_path"] = c->path;
					params[type + "_" + c->name + "_bundled"] = c->bundled ? "1":"0";
				}
			}
		}
		return params;
	}
#endif
}
