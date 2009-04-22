/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "stdlib.h"
#include <process.h>
#include <utils.h>

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

using kroll::Application;
using kroll::KComponent;
using std::string;
using std::vector;
using namespace kroll;

//
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

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

Application* app;
string applicationHome;
string updateFile;

// If this is an application install -- versus an update
// or just missing modules, record where it installed to.
string appInstallPath;


int argc = 0;
char **argv = NULL;

inline void ShowError(string msg, bool fatal=false)
{
	std::cerr << "Error: " << msg << std::endl;
	MessageBox(NULL, msg.c_str(), "Application Error", MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
	if (fatal)
		exit(1);
}

bool RunInstaller(vector<KComponent*> missing)
{
	string exec = kroll::FileUtils::Join(app->path.c_str(), "installer", "Installer.exe", NULL);
	if (!kroll::FileUtils::IsFile(exec))
	{
		ShowError("Missing installer and application has additional modules that are needed.");
		return false;
	}

	vector<string> args;
	args.push_back("-appPath");
	args.push_back(applicationHome);
	args.push_back("-runtimeHome");
	args.push_back(app->runtimeHomePath);
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

	// A little bit of ugliness goes a long way:
	// Use ShellExecuteEx here with the undocumented runas verb
	// so that we can execute the installer executable and have it
	// properly do the UAC thing. Why isn't this in the API?
	string paramString = "";
	for (size_t i = 0; i < args.size(); i++)
	{
		paramString.append(" \"");
		paramString.append(args.at(i));
		paramString.append("\"");
	}

	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = "runas";
	ShExecInfo.lpFile = exec.c_str();
	ShExecInfo.lpParameters = paramString.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;	
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

	// Ugh. Now we need to figure out where the app installer installed
	// to. We would normally use stdout, but we had to execute with
	// an expensive call to ShellExecuteEx, so we're just going to read
	// the information from a file.
	if (!app->IsInstalled())
	{
		string fpath = FileUtils::Join(app->path.c_str(), ".installedto", NULL);
		printf("fpath: %s\n", fpath.c_str());

		// The user probably cancelled -- don't show an error
		if (!FileUtils::IsFile(fpath))
			return false;

		std::ifstream file(fpath.c_str());
		if (file.bad() || file.fail() || file.eof())
		{
			ShowError("Application installed failed.");
			return false; // Don't show further errors
		}
		std::getline(file, appInstallPath);
		appInstallPath = FileUtils::Trim(appInstallPath);

		Application* newapp = BootUtils::ReadManifest(appInstallPath);
		if (newapp == NULL)
		{
			ShowError("Application installed failed.");
			return false; // Don't show further errors
		}
		else
		{
			app = newapp;
		}
	}

	return true;
}

std::string GetApplicationHomePath()
{
	char path[MAX_PATH];
	int size = GetModuleFileName(GetModuleHandle(NULL), (char*)path, MAX_PATH);
	if (size>0)
	{
		path[size] = '\0';
	}
	return FileUtils::Dirname(string(path));
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

vector<KComponent*> FindModules()
{
	vector<string> runtimeHomes;

	// Add the default runtime home for now, later this
	// might be a list of possible locations, like on Linux
	runtimeHomes.push_back(FileUtils::GetDefaultRuntimeHomeDirectory());

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
		moduleList << module->path << ";";
	}

	// Registration-free COM requires that our manifest point to a DLL
	// in a subdirectory of the executable path. Thus we need to copy the
	// runtime WebKit.dll into the runtime directory of our application home.
	std::string appRuntimePath = FileUtils::Join(app->path.c_str(), "runtime", NULL);
	std::string appWebkitDll = FileUtils::Join(appRuntimePath.c_str(), "WebKit.dll", NULL);
	if (!kroll::FileUtils::IsFile(appWebkitDll))
	{
		kroll::FileUtils::CopyRecursive(app->runtime->path, appRuntimePath);
	}

	// Add runtime path and all module paths to PATH
	string path = appRuntimePath + ";";
	path.append(app->runtime->path + ";");
	path.append(moduleList.str() + ";");
	if (EnvironmentUtils::Has("PATH"))
	{
		path.append(EnvironmentUtils::Get("PATH"));
	}
	EnvironmentUtils::Set("PATH", path);

	EnvironmentUtils::Set("KR_FORK", "YES");
	EnvironmentUtils::Set("KR_HOME", app->path);
	EnvironmentUtils::Set("KR_RUNTIME", app->runtime->path);
	EnvironmentUtils::Set("KR_MODULES", moduleList.str());
	EnvironmentUtils::Set("KR_RUNTIME_HOME", app->runtimeHomePath);
	EnvironmentUtils::Set("KR_APP_GUID", app->guid);
	EnvironmentUtils::Set("KR_APP_ID", app->id);

	// If this was a full app install, we need to execute the newly
	// installed executable as opposed to this one -- which is in a
	// temporary directory somewhere.
	string executable = argv[0];
	if (!appInstallPath.empty())
	{
		executable = FileUtils::Basename(executable);
		executable = FileUtils::Join(appInstallPath.c_str(), executable.c_str(), NULL);
	}

	_execvp(executable.c_str(), argv);
	return 0;
}

typedef int Executor(HINSTANCE, int, const char **);
int StartHost()
{
	string runtimePath = EnvironmentUtils::Get("KR_RUNTIME");
	std::string khost = FileUtils::Join(runtimePath.c_str(), "khost.dll", NULL);

	// now we need to load the host and get 'er booted
	if (!FileUtils::IsFile(khost))
	{
		ShowError(string("Couldn't find required file: ") + khost);
		return __LINE__;
	}

	HMODULE dll = LoadLibrary(khost.c_str());
	if (!dll)
	{
		char msg[MAX_PATH];
		sprintf_s(msg,"Couldn't load file: %s, error: %d", khost.c_str(), GetLastError());
		ShowError(msg);
		return __LINE__;
	}
	Executor *executor = (Executor*)GetProcAddress(dll, "Execute");
	if (!executor)
	{
		ShowError(string("Invalid entry point for") + khost);
		return __LINE__;
	}

	return executor(::GetModuleHandle(NULL), argc,(const char**)argv);
}

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int __argc, const char* __argv[])
#endif
{
	argc = __argc;
	argv = (char**) __argv;

	int rc;
	if (!EnvironmentUtils::Has("KR_FORK"))
	{
		rc = Bootstrap();
	}
	else
	{
		EnvironmentUtils::Unset("KR_FORK");
		rc = StartHost();
	}

	#ifdef DEBUG
	std::cout << "return code: " << rc << std::endl;
	#endif
	return rc;
}
