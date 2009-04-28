/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "boot.h"

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

	// If this is an application install -- versus an update
	// or just missing modules, record where it installed to
	// Windows does an actual copy-files type install currently.
	string appInstallPath;

	inline void ShowError(string msg, bool fatal)
	{
		std::cerr << "Error: " << msg << std::endl;
		MessageBox(NULL, msg.c_str(), "Application Error", MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
		if (fatal)
			exit(1);
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

	bool RunInstaller(vector<SharedDependency> missing)
	{
		string exec = FileUtils::Join(
			app->path.c_str(),
			"installer",
			"Installer.exe", NULL);

		if (!FileUtils::IsFile(exec))
		{
			ShowError("Missing installer and application has additional modules that are needed.");
			return false;
		}

		vector<string> args;
		args.push_back("-appPath");
		args.push_back(applicationHome);
		args.push_back("-exePath");
		args.push_back(argv[0]);
		if (!updateFile.empty())
		{
			args.push_back("-updateFile");
			args.push_back(updateFile);
		}
	
		vector<string> jobs;
		vector<SharedDependency*>::iterator mi = missing.begin();
		while (mi != missing.end())
		{
			SharedDependency d = *di++;
			string url = app->GetURLForDependency(d);
			jobs.push_back(url);
		}
	
		// A little bit of ugliness goes a long way:
		// Use ShellExecuteEx here with the undocumented runas verb
		// so that we can execute the installer executable and have it
		// properly do the UAC thing. Why isn't this in the API?
	
		// More ugliness: The path length for ShellExecuteEx is limited so just
		// pass a file name containing all the download jobs. :(
		string tempdir = FileUtils::GetTempDirectory();
		string jobsFile = FileUtils::Join(tempdir.c_str(), "jobs", NULL);
		if (jobs.size() > 0)
		{
			FileUtils::CreateDirectory(tempdir);
			std::ofstream outfile(jobsFile.c_str());
			for (size_t i = 0; i < jobs.size(); i++)
			{
				outfile << jobs[i] << "\n";
			}
			outfile.close();
			args.push_back(jobsFile);
		}
	
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

		if (FileUtils::IsDirectory(tempdir))
		{
			FileUtils::DeleteDirectory(tempdir);
		}

		// Ugh. Now we need to figure out where the app installer installed
		// to. We would normally use stdout, but we had to execute with
		// an expensive call to ShellExecuteEx, so we're just going to read
		// the information from a file.
		if (!app->IsInstalled())
		{
			string installedToFile = FileUtils::Join(app->path.c_str(), ".installedto", NULL);
			// The user probably cancelled -- don't show an error
			if (!FileUtils::IsFile(installedToFile))
				return false;

			std::ifstream file(installedToFile.c_str());
			if (file.bad() || file.fail() || file.eof())
			{
				DeleteFile(installedToFile.c_str());
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
			DeleteFile(installedToFile.c_str());
		}

		return true;
	}

	void BootstrapPlatformSpecific(string moduleList)
	{
		moduleList = app->runtime->path + ";" + moduleList;

		// Add runtime path and all module paths to PATH
		string path = moduleList;
		string currentPath = EnvironmentUtils::Get("PATH");
		if (!currentPath.empty())
		{
			path = path + ":" + currentPath;
		}
		EnvironmentUtils::Set("PATH", path);
	}

	string Blastoff
	{
		// If this was a full app install, we need to execute the newly
		// installed executable as opposed to this one -- which is in a
		// temporary directory somewhere.
		string executable = argv[0];
		if (!appInstallPath.empty())
		{
			executable = FileUtils::Basename(executable);
			executable = FileUtils::Join(appInstallPath.c_str(), executable.c_str(), NULL);
		}

		_execv(executable.c_str(), argv);

		// If we get here an error happened with the execv 
		return strerror(errno);
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
}

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int __argc, const char* __argv[])
#endif
{
	KrollBoot::argc = __argc;
	KrollBoot::argv = (const char**) __argv;

#ifdef USE_BREAKPAD
	if (argc > 2 && !strcmp(CRASH_REPORT_OPT, argv[1]))
	{
		return KrollBoot::SendCrashReport();
	}

	// Don't install a handler if we are just handling an error (above).
	string dumpPath = "/tmp";
	breakpad = new google_breakpad::ExceptionHandler(
		dumpPath,
		NULL,
		KrollBoot::HandleCrash,
		NULL,
		true);
#endif

	int rc;
	if (!EnvironmentUtils::Has(BOOTSTRAP_ENV))
	{
		return Bootstrap();
	}
	else
	{
		EnvironmentUtils::Unset(BOOTSTRAP_ENV);
		return StartHost();
	}

	return rc;
}
