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
			0,
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
		if (realPath != 0 && FileUtils::IsFile(realPathStr))
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

		string path(moduleList);
		string current(EnvironmentUtils::Get("LD_LIBRARY_PATH"));
		EnvironmentUtils::Set("KR_ORIG_LD_LIBRARY_PATH", current);

		if (!current.empty())
			path.append(":" + current);

		EnvironmentUtils::Set("LD_LIBRARY_PATH", path);
	}

	string Blastoff()
	{
		// Ensure that the argument list is NULL terminated
		char** myargv = (char **) calloc(sizeof(char *), argc + 1);
		memcpy(myargv, argv, sizeof(char*) * (argc + 1));
		myargv[argc] = 0;

		execv(argv[0], (char* const*) argv);

		// If we get here an error happened with the execv 
		return strerror(errno);
	}


	typedef int Executor(int argc, const char **argv);
	int StartHost()
	{
		const char* runtimePath = getenv("KR_RUNTIME");
		if (!runtimePath)
			return __LINE__;

		// now we need to load the host and get 'er booted
		string khost = FileUtils::Join(runtimePath, "libkhost.so", 0);
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
			app->path.c_str(), "installer", "installer", 0);
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
}

int main(int argc, const char* argv[])
{
	KrollBoot::argc = argc;
	KrollBoot::argv = argv;

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

