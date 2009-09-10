/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"

namespace UTILS_NS
{
namespace BootUtils
{
	vector<string>& GetComponentSearchPaths()
	{
		static std::vector<std::string> componentSearchPaths;
		if (componentSearchPaths.empty())
		{
			// Allow the user to force an override to the runtime home by setting the
			// appropriate environment variable -- this will be the first path searched
			if (EnvironmentUtils::Has("KR_SEARCH_PATH"))
			{
				componentSearchPaths.push_back(EnvironmentUtils::Get("KR_SEARCH_PATH"));
			}

			componentSearchPaths.push_back(FileUtils::GetUserRuntimeHomeDirectory());
			componentSearchPaths.push_back(FileUtils::GetSystemRuntimeHomeDirectory());
		}
		return componentSearchPaths;
	}

	bool RunInstaller(
		vector<SharedDependency> missing,
		SharedApplication application,
		std::string updateFile,
		std::string installerPath,
		bool quiet,
		bool forceInstall)
	{
		if (installerPath.empty())
		{
			installerPath = application->path;
		}

		string exec = FileUtils::Join(
			installerPath.c_str(),
			"installer",
			"Installer App.app",
			"Contents", 
			"MacOS",
			"Installer App", NULL);

		if (!FileUtils::IsFile(exec))
		{
			return false;
		}

		vector<string> args;
		args.push_back("-appPath");
		args.push_back(application->path);

		if (!updateFile.empty())
		{
			args.push_back("-updateFile");
			args.push_back(updateFile);
		}

		if (quiet)
		{
			args.push_back("-quiet");
		}

		std::vector<SharedDependency>::iterator di = missing.begin();
		while (di != missing.end())
		{
			SharedDependency d = *di++;
			string url = application->GetURLForDependency(d);
			args.push_back(url);
		}

		FileUtils::RunAndWait(exec, args);
		return true;
	}
}
}
