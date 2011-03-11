/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "boot.h"
#include "popup_dialog_win32.h"
#include <process.h>
#include <windows.h>
using std::wstring;

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

namespace KrollBoot
{
	extern string applicationHome;
	extern string updateFile;
	extern SharedApplication app;
	extern int argc;
	extern const char** argv;
	
	inline void ShowError(string msg, bool fatal)
	{
		wstring wideMsg(L"Error: ");
		wideMsg.append(KrollUtils::UTF8ToWide(msg));
		wstring wideAppName = KrollUtils::UTF8ToWide(GetApplicationName());

		MessageBoxW(0, wideMsg.c_str(), wideAppName.c_str(), MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
		if (fatal)
			exit(1);
	}

	string GetApplicationHomePath()
	{
		wchar_t widePath[MAX_PATH];
		int size = GetModuleFileNameW(GetModuleHandle(0), widePath, MAX_PATH - 1);
		if (size > 0)
		{
			widePath[size] = '\0';
			string path = KrollUtils::WideToUTF8(widePath);
			return FileUtils::Dirname(path);
		}
		else
		{
			ShowError("Could not determine application path.", true);
		}
	}

	bool IsWindowsXP()
	{
		OSVERSIONINFO osVersion;
		osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osVersion);
		return osVersion.dwMajorVersion == 5;
	}

	void BootstrapPlatformSpecific(string path)
	{
		// Add runtime path and all module paths to PATH
		path = app->runtime->path + ";" + path;
		string currentPath(EnvironmentUtils::Get("PATH"));
		EnvironmentUtils::Set("KR_ORIG_PATH", currentPath);

		// make sure the runtime folder is used before system DLL directories
		SetDllDirectoryW(KrollUtils::UTF8ToWide(app->runtime->path).c_str());
		
		if (!currentPath.empty())
			path = path + ";" + currentPath;
		EnvironmentUtils::Set("PATH", path);
	}

	string Blastoff()
	{
		// Windows boot does not normally need to restart itself,  so just
		// launch the host here and exit with the appropriate return value.

		// This may have been an install, so ensure that KR_HOME is correct
		EnvironmentUtils::Set("KR_HOME", app->path);
		exit(KrollBoot::StartHost());
	}

	static HMODULE SafeLoadRuntimeDLL(string& path)
	{
		if (!FileUtils::IsFile(path))
		{
			ShowError(string("Couldn't find required file: ") + path);
			return false;
		}

		wstring widePath(KrollUtils::UTF8ToWide(path));
		HMODULE module = LoadLibraryExW(widePath.c_str(),
			0, LOAD_WITH_ALTERED_SEARCH_PATH);
		if (!module)
		{
			string msg("Couldn't load file (");
			msg.append(path);
			msg.append("): ");
			msg.append(KrollUtils::Win32Utils::QuickFormatMessage(GetLastError()));
			ShowError(msg);
		}

		return module;
	}

	typedef int Executor(HINSTANCE, int, const char **);
	int StartHost()
	{
		string runtimePath(EnvironmentUtils::Get("KR_RUNTIME"));
		string dll(FileUtils::Join(runtimePath.c_str(), "khost.dll", 0));
		HMODULE khost = SafeLoadRuntimeDLL(dll);
		if (!khost)
			return __LINE__;

		Executor *executor = (Executor*) GetProcAddress(khost, "Execute");
		if (!executor)
		{
			ShowError(string("Invalid entry point 'Execute' in khost.dll"));
			return __LINE__;
		}

		return executor(::GetModuleHandle(NULL), argc, (const char**)argv);
	}

	bool RunInstaller(vector<SharedDependency> missing, bool forceInstall)
	{

		string installer(FileUtils::Join(app->path.c_str(), "installer", "installer.exe", 0));
		if (!FileUtils::IsFile(installer))
		{
			ShowError("Missing installer and application has additional modules that are needed.");
			return false;
		}
		return BootUtils::RunInstaller(missing, app, updateFile, "", false, forceInstall);
	}

	string GetApplicationName()
	{
		if (!app.isNull())
		{
			return app->name.c_str();
		}
		return PRODUCT_NAME;
	}
}

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int __argc, const char* __argv[])
#endif
{
	KrollBoot::argc = __argc;
	KrollBoot::argv = (const char**) __argv;

	return KrollBoot::Bootstrap();
}
