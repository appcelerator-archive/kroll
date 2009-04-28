/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include "boot.h"

#ifdef USE_BREAKPAD
#include "client/linux/handler/exception_handler.h"
#include "common/linux/http_upload.h"
#endif

namespace KrollBoot
{
	extern string applicationHome;
	extern string updateFile;
	extern SharedApplication app;
	extern int argc;
	extern const char** argv;

	void ShowError(string error, bool fatal)
	{
		std::cout << error << std::endl;
		if (fatal)
			exit(1);
	}

	string GetApplicationHomePath()
	{
		char* buffer = (char*) alloca(sizeof(char) * PATH_MAX);
		char* realPath = realpath(argv[0], buffer);
		string realPathStr = realPath;
		if (realPath != NULL && FileUtils::IsFile(realPathStr))
		{
			return dirname(realPath);
		}
		else
		{
			return FileUtils::GetApplicationDirectory();
		}
	}

	bool RunInstaller(vector<SharedDependency> missing)
	{
		string exec = kroll::FileUtils::Join(
			app->path.c_str(), "installer", "installer", NULL);

		if (!kroll::FileUtils::IsFile(exec))
		{
			ShowError("Missing installer and application has additional modules that are needed.");
			return false;
		}

		vector<string> args;
		args.push_back("--apppath");
		args.push_back(app->path);
		if (!updateFile.empty())
		{
			args.push_back("--updatefile");
			args.push_back(updateFile);
		}

		std::vector<SharedDependency>::iterator di = missing.begin();
		while (di != missing.end())
		{
			SharedDependency d = *di++;
			string url = app->GetURLForDependency(d);
			args.push_back(url);
		}

		kroll::FileUtils::RunAndWait(exec, args);
	}

	void BootstrapPlatformSpecific(string moduleList)
	{
		moduleList = app->runtime->path + ":" + moduleList;

		string path = moduleList;
		string current = EnvironmentUtils::Get("LD_LIBRARY_PATH");
		if (!current.empty())
		{
			path.append(":" + current);
		}
		EnvironmentUtils::Set("LD_LIBRARY_PATH", path);
	}

	string Blastoff()
	{
		// Ensure that the argument list is NULL terminated
		char** myargv = (char **) calloc(sizeof(char *), argc + 1);
		memcpy(myargv, argv, sizeof(char*) * (argc + 1));
		myargv[argc] = NULL;

		execv(argv[0], (char* const*) argv);

		// If we get here an error happened with the execv 
		return strerror(errno);
	}


	typedef int Executor(int argc, const char **argv);
	int StartHost()
	{
		const char* runtimePath = getenv("KR_RUNTIME");
		if (runtimePath == NULL)
			return __LINE__;

		// now we need to load the host and get 'er booted
		string khost = FileUtils::Join(runtimePath, "libkhost.so", NULL);
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

		Executor* executor = (Executor*) dlsym(lib, "Execute");
		if (!executor)
		{
			string msg = string("Invalid entry point for") + khost;
			ShowError(msg);
			return __LINE__;
		}

		return executor(argc, argv);
	}

#ifdef USE_BREAKPAD
	static google_breakpad::ExceptionHandler* breakpad;

	char breakpadCallBuffer[PATH_MAX];
	bool HandleCrash(
		const char* dump_path,
		const char* id,
		void* context,
		bool succeeded)
	{
		if (succeeded)
		{
			snprintf(breakpadCallBuffer, PATH_MAX - 1,
				 "%s %s %s %s", argv[0], CRASH_REPORT_OPT, dump_path, id);
			system(breakpadCallBuffer);
		}
#ifdef DEBUG
		return false
#else
		return true;
#endif
	}
	
	int SendCrashReport(int argc, const char* argv[])
	{
		string url = STRING(CRASH_REPORT_URL);
		const std::map<string, string> parameters = get_parameters(argc, argv);
		string upload_file = argv[2];
		string filePartName = "dump";
		string proxy;
		string proxyUserPassword;
		string responseBody;
		string errorDescription;

		bool success google_breakpad::HTTPUpload::SendRequest(
			url,
			parameters,
			dumpFilePath.c_str(),
			filePartName,
			proxy,
			proxyUserPassword,
			&responseBody,
			&errorDescription
		);

		if (!success)
		{
			GtkWidget* dialog = gtk_message_dialog_new(
				NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				"Error uploading crash dump: %s",
				errorDescription.c_str());
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
		}
		return 1;
	}
#endif
}

int main(int argc, const char* argv[])
{
	KrollBoot::argc = argc;
	KrollBoot::argv = argv;
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

	if (!EnvironmentUtils::Has(BOOTSTRAP_ENV))
	{
		return KrollBoot::Bootstrap();
	}
	else
	{
		EnvironmentUtils::Unset(BOOTSTRAP_ENV);
		return KrollBoot::StartHost();
	}
}

