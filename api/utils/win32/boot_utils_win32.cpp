/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"
#include <sstream>

using std::wstring;

namespace UTILS_NS
{
namespace BootUtils
{
	static bool IsWindowsXP()
	{
		OSVERSIONINFO osVersion;
		osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osVersion);
		return osVersion.dwMajorVersion == 5;
	}
	
	vector<string>& GetComponentSearchPaths()
	{
		static std::vector<std::string> componentSearchPaths;
		if (componentSearchPaths.empty())
		{
			// Allow the user to force an override to the runtime home by setting the
			// appropriate environment variable -- this will be the first path searched
			if (EnvironmentUtils::Has("KR_SEARCH_PATH"))
				componentSearchPaths.push_back(EnvironmentUtils::Get("KR_SEARCH_PATH"));

			componentSearchPaths.push_back(FileUtils::GetUserRuntimeHomeDirectory());
			componentSearchPaths.push_back(FileUtils::GetSystemRuntimeHomeDirectory());
		}
		return componentSearchPaths;
	}

	bool RunInstaller(vector<SharedDependency> missing,
		SharedApplication application, string updateFile,
		string installerPath, bool quiet, bool forceInstall)
	{
		if (installerPath.empty())
		{
			installerPath = application->path;
		}

		string msiName = application->name + ".msi";
		string installer(FileUtils::Join(installerPath.c_str(), 
			"installer", msiName.c_str(), 0));
		string msiExec(FileUtils::Join("C:", "Windows", "system32", "msiexec.exe", NULL));
		if (!FileUtils::IsFile(installer) && !FileUtils::IsFile(msiExec))
		{
			return false;
		}
		
		std::wstring commandLine(UTF8ToWide(msiExec));
		commandLine += L" /I \"";
		commandLine += UTF8ToWide(installer);
		commandLine += L"\" ";
		
		commandLine += L"REINSTALL=ALL REINSTALLMODE=vomus";
		commandLine += L" APP_UPDATE_MANIFEST=\"";
		if (!updateFile.empty())
		{
			commandLine += UTF8ToWide(updateFile);
		}
		else
		{
			commandLine += UTF8ToWide(application->manifestPath);
		}
		commandLine += L"\"";
		std::wcout << commandLine << std::endl;
		
		STARTUPINFO startupInfo;
		startupInfo.cb          = sizeof(STARTUPINFO);
		startupInfo.lpReserved  = NULL;
		startupInfo.lpDesktop   = NULL;
		startupInfo.lpTitle     = NULL;
		startupInfo.dwFlags     = STARTF_FORCEOFFFEEDBACK | STARTF_USESTDHANDLES;
		startupInfo.cbReserved2 = 0;
		startupInfo.lpReserved2 = NULL;

		PROCESS_INFORMATION processInfo;
		BOOL rc = CreateProcessW(NULL,
			(LPWSTR) commandLine.c_str(),
			NULL,
			NULL,
			TRUE,
			0,
			NULL,
			NULL,
			&startupInfo,
			&processInfo);
		
		DWORD returnCode;
		if (rc)
		{
			CloseHandle(processInfo.hThread);
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			GetExitCodeProcess(processInfo.hProcess, &returnCode);
			CloseHandle(processInfo.hProcess);
		}

		if (returnCode != 0) {
			return false;
		}
		return true;
	}
}
}
