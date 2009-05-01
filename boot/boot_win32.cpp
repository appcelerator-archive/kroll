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

#ifdef USE_BREAKPAD
#include "client/windows/handler/exception_handler.h"
#include "common/windows/http_upload.h"
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
		MessageBoxA(NULL, msg.c_str(), "Application Error", MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
		if (fatal)
			exit(1);
	}

	std::string GetApplicationHomePath()
	{
		char path[MAX_PATH];
		int size = GetModuleFileNameA(GetModuleHandle(NULL), (char*)path, MAX_PATH);
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
		vector<SharedDependency>::iterator mi = missing.begin();
		while (mi != missing.end())
		{
			SharedDependency d = *mi++;
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
		printf("%s\n", paramString.c_str());
	
		SHELLEXECUTEINFOA ShExecInfo = {0};
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = "runas";
		ShExecInfo.lpFile = exec.c_str();
		ShExecInfo.lpParameters = paramString.c_str();
		ShExecInfo.lpDirectory = NULL;
		ShExecInfo.nShow = SW_SHOW;
		ShExecInfo.hInstApp = NULL;	
		ShellExecuteExA(&ShExecInfo);
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
				DeleteFileA(installedToFile.c_str());
				ShowError("Application installed failed.");
				return false; // Don't show further errors
			}
			std::getline(file, appInstallPath);
			appInstallPath = FileUtils::Trim(appInstallPath);

			SharedApplication newapp = Application::NewApplication(appInstallPath);
			if (newapp.isNull())
			{
				ShowError("Application installed failed.");
				return false; // Don't show further errors
			}
			else
			{
				app = newapp;
			}
			DeleteFileA(installedToFile.c_str());
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

	string Blastoff()
	{
		// Windows boot does not need to restart itself, so just launch
		// the host here and exit with the appropriate return value.
		EnvironmentUtils::Unset(BOOTSTRAP_ENV);
		exit(KrollBoot::StartHost());
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
	
		HMODULE dll = LoadLibraryA(khost.c_str());
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

#ifdef USE_BREAKPAD
	static google_breakpad::ExceptionHandler* breakpad;
	extern string dumpFilePath;

	char breakpadCallBuffer[MAX_PATH];
  	bool HandleCrash(
		const wchar_t* dumpPath,
		const wchar_t* id,
		void* context,
		EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion,
		bool succeeded)
	{
		if (succeeded)
		{
			STARTUPINFOA startupInfo = {0};
			startupInfo.cb = sizeof(startupInfo);
			PROCESS_INFORMATION processInformation;
			_snprintf_s(breakpadCallBuffer, MAX_PATH, MAX_PATH - 1,
				 "%s %s %S %S", argv[0], CRASH_REPORT_OPT, dumpPath, id);
			CreateProcessA(
				NULL,
				breakpadCallBuffer,
				NULL,
				NULL,
				FALSE,
				0,
				NULL,
				NULL,
				&startupInfo,
				&processInformation);
		}
#ifdef DEBUG
		return false;
#else
		return true;
#endif
	}

	wstring StringToWString(string in)
	{
		wstring out(in.length(), L' ');
		copy(in.begin(), in.end(), out.begin());
		return out; 
	}

	map<wstring, wstring> GetCrashReportParametersW()
	{
		map<wstring, wstring> paramsW;
		map<string, string> params = GetCrashReportParameters();
		map<string, string>::iterator i = params.begin();
		while (i != params.end())
		{
			wstring key = StringToWString(i->first);
			wstring val = StringToWString(i->second);
			i++;

			paramsW[key] = val;
		}
		return paramsW;
	}
	
	int SendCrashReport()
	{
		std::string title = PRODUCT_NAME" has crashed";
		std::string msg = PRODUCT_NAME" has crashed. Do you want to send a crash report?";

		Win32PopupDialog popupDialog(NULL);
		popupDialog.SetTitle(title);
		popupDialog.SetMessage(msg);
		popupDialog.SetShowCancelButton(true);
		if (popupDialog.Show() != IDYES)
		{
			return 1;
		}

		wstring url = StringToWString(STRING(CRASH_REPORT_URL));
		const std::map<wstring, wstring> parameters = GetCrashReportParametersW();
		wstring dumpFilePathW = StringToWString(dumpFilePath);
		wstring responseBody;
		int responseCode;

		bool success = google_breakpad::HTTPUpload::SendRequest(
			url,
			parameters,
			dumpFilePathW.c_str(),
			L"dump",
			NULL,
			&responseBody,
			&responseCode);

		if (!success)
		{
			ShowError("Error uploading crash dump.");
			return 2;
		}
		return 0;
	}
#endif
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
	// Don't install a handler if we are just handling an error.
	if (__argc > 2 && !strcmp(CRASH_REPORT_OPT, __argv[1]))
	{
		return KrollBoot::SendCrashReport();
	}

	wchar_t tempPath[MAX_PATH];
	GetTempPathW(MAX_PATH, tempPath);
	KrollBoot::breakpad = new google_breakpad::ExceptionHandler(
		tempPath,
		NULL,
		KrollBoot::HandleCrash,
		NULL,
		google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif
	return KrollBoot::Bootstrap();
}
