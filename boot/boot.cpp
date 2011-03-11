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
		string file = FileUtils::GetApplicationDataDirectory(GetApplicationName());
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
		if (!updateFile.empty())
		{
			FileUtils::DeleteFile(updateFile);
		}

		string error = Blastoff();
		// If everything goes correctly, we should never get here
		error = string("Launching application failed: ") + error;
		ShowError(error, false);
		return __LINE__;
	}
}
