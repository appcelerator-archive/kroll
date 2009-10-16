/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include <gtk/gtk.h>
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
		gtk_init(&argc, (char***) &argv);
		GtkWidget* dialog = gtk_message_dialog_new(
			NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"%s",
			error.c_str());
		gtk_window_set_title(GTK_WINDOW(dialog), GetApplicationName().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
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
			return FileUtils::GetExecutableDirectory();
		}
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

	bool RunInstaller(vector<SharedDependency> missing, bool forceInstall)
	{
		string exec = FileUtils::Join(
			app->path.c_str(), "installer", "installer", NULL);
		if (!FileUtils::IsFile(exec))
		{
			ShowError("Missing installer and application has additional modules that are needed.");
			return false;
		}
		return BootUtils::RunInstaller(missing, app, updateFile);
	}

	string GetApplicationName()
	{
		if (!app.isNull())
		{
			return app->name.c_str();
		}
		return PRODUCT_NAME;
	}

#ifdef USE_BREAKPAD
	static google_breakpad::ExceptionHandler* breakpad;
	extern string dumpFilePath;

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
				 "\"%s\" %s \"%s\" %s&", argv[0], CRASH_REPORT_OPT, dump_path, id);
			system(breakpadCallBuffer);
		}
#ifdef DEBUG
		return false;
#else
		return true;
#endif
	}

	int SendCrashReport()
	{
		gtk_init(&argc, (char***) &argv);

		InitCrashDetection();
		std::string title = GetCrashDetectionTitle();
		std::string msg = GetCrashDetectionHeader();
		msg.append("\n\n");
		msg.append(GetCrashDetectionMessage());

		string url = string("https://") + CRASH_REPORT_URL;
		const std::map<string, string> parameters = GetCrashReportParameters();

		GtkWidget* dialog = gtk_message_dialog_new(
			NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_NONE,
			"%s",
			msg.c_str());
		gtk_dialog_add_buttons(GTK_DIALOG(dialog),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			"Send Report", GTK_RESPONSE_OK,
			NULL);
		gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());
		int response = gtk_dialog_run(GTK_DIALOG(dialog));
		if (response != GTK_RESPONSE_OK)
		{
			return __LINE__;
		}

		string filePartName = "dump";
		string proxy;
		string proxyUserPassword;
		string responseBody;
		string errorDescription;
		bool success = google_breakpad::HTTPUpload::SendRequest(
			url, parameters, dumpFilePath.c_str(), filePartName,
			proxy, proxyUserPassword, &responseBody, &errorDescription);

		if (!success)
		{
			ShowError(string("Error uploading crash dump: ") + errorDescription);
			return __LINE__;
		}
		return 0;
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
	KrollBoot::breakpad = new google_breakpad::ExceptionHandler(
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

