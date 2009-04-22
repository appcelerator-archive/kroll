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
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <utils.h>

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

using namespace kroll;
using std::string;
using kroll::FileUtils;
using kroll::BootUtils;
using kroll::Application;
using kroll::KComponent;

#define MODULE_DIR "modules"
#define RUNTIME_DIR "runtime"

typedef int Executor(int argc, const char **argv);

typedef struct InstallLocation
{
	std::string runtimeHome;
	std::string runtime;
	std::string modules;
} InstallLocation;

class Boot
{
	public:
	Application* app;
	std::string applicationPath;
	std::string updateFile;
	std::string installerPath;

	/* Default runtime base path or, if a runtime is found, that runtime home path */
	std::string activeRuntimeHome;

	/* Potential paths for installed modules and runtime */
	std::vector<InstallLocation*> installLocations;

	/* Paths for bundled modules and runtime */
	std::string bundledModulePath;
	std::string bundledRuntimePath;

	void AddInstallLocation(std::string path)
	{
		InstallLocation* location = new InstallLocation;
		location->runtimeHome = path;
		location->runtime = FileUtils::Join(path.c_str(), RUNTIME_DIR, "linux", NULL);
		location->modules = FileUtils::Join(path.c_str(), MODULE_DIR, "linux", NULL);
		this->installLocations.push_back(location);
	}


	void SetupPaths(const char* argv)
	{
		char* buffer = (char*) alloca(sizeof(char) * PATH_MAX);
		char* real_path = realpath(argv, buffer);
		if (real_path == NULL)
		{
			this->applicationPath = FileUtils::GetApplicationDirectory();
		}
		else
		{
			struct stat statbuf;
			int result = stat(real_path, &statbuf);
			if (result == 0)
				this->applicationPath = std::string(dirname(real_path));
			else
				this->applicationPath = FileUtils::GetApplicationDirectory();
		}

		const char* capplicationPath = applicationPath.c_str();
		this->installerPath = FileUtils::Join(capplicationPath, "installer", NULL);

		// Allow the user to force an override to the runtime home by setting the
		// appropriate environment variable -- this will be the first path searched
		if (EnvironmentUtils::Has("KR_RUNTIME_HOME"))
			this->AddInstallLocation(EnvironmentUtils::Get("KR_RUNTIME_HOME"));

		// Kroll runtime and modules will located by searching the following paths in order:
		// 1. ~/.PRODUCT_NAME (eg. ~/.titanium)
		// 2. /opt/PRODUCT_NAME (default runtime base path for system-wide installation)
		// 3. /usr/local/lib/PRODUCT_NAME
		// 4. /usr/lib/PRODUCT_NAME
		// We can't use FileUtils::GetSystemRuntimeHomeDirectory() because it only locates
		// the first one of these directories that exists

		string pname = PRODUCT_NAME;
		std::transform(pname.begin(), pname.end(), pname.begin(), tolower);

		this->AddInstallLocation(FileUtils::GetUserRuntimeHomeDirectory());
		this->AddInstallLocation(std::string("/opt") + pname);
		this->AddInstallLocation(std::string("/usr/local/lib") + pname);
		this->AddInstallLocation(std::string("/usr/lib/") + pname);

		this->bundledModulePath = FileUtils::Join(capplicationPath, MODULE_DIR, NULL);
		this->bundledRuntimePath = FileUtils::Join(capplicationPath, RUNTIME_DIR, NULL);

		// The installer directory could previously contain bundled runtime / modules.
		// This location is no longer supported for Linux.
	}

	void FindUpdate()
	{
		// Search for an update file in the application data  directory.
		// It will be placed there by the update service. If it exists
		// and the version in the update file is greater than the
		// current app version, we want to force an update.
		std::string file = FileUtils::GetApplicationDataDirectory(app->id);
		file = FileUtils::Join(file.c_str(), UPDATE_FILENAME, NULL);
		if (FileUtils::IsFile(file))
		{
			Application* update = BootUtils::ReadManifestFile(file, app->path);
			if (BootUtils::CompareVersions(update->version, app->version) > 0)
			{
				this->app = update;
				this->updateFile = file;
			}
		}
	}

	Boot(const char* firstArg) :
		app(NULL),
		updateFile(std::string())
	{
		this->SetupPaths(firstArg);

		this->app = BootUtils::ReadManifest(this->applicationPath);
		if (this->app == NULL)
			throw std::string("Application packaging error. The application "
				"manifest was not found in the correct location.");

		this->FindUpdate();
	}

	~Boot()
	{
		if (app != NULL)
			delete this->app;

		std::vector<InstallLocation*>::iterator li = this->installLocations.begin();
		while (li != this->installLocations.end())
		{
			InstallLocation* l = *li;
			li = this->installLocations.erase(li);
			delete l;
		}
	}


	std::vector<KComponent*> FindModules()
	{
		std::vector<KComponent*> unresolved;

		// Find the runtime module
		if (!this->FindRuntime(this->app->runtime))
		{
			unresolved.push_back(this->app->runtime);
		}

		// Find all regular modules
		std::vector<KComponent*>::iterator i = this->app->modules.begin();
		while (i != app->modules.end())
		{
			KComponent* m = *i++;
			if (!this->FindModule(m))
			{
				unresolved.push_back(m);
			}
		}

		if (unresolved.size() > 0)
		{
			std::vector<KComponent*>::iterator dmi = unresolved.begin();
			while (dmi != unresolved.end())
			{
				KComponent* m = *dmi++;
				std::cout << "Unresolved: " << m->name << std::endl;
			}
			std::cout << "---" << std::endl;
		}

		return unresolved;
	}

	bool FindModule(KComponent *m)
	{
		// Try to find the bundled version of this module.
		std::string path = FileUtils::Join(this->bundledModulePath.c_str(), m->name.c_str(), NULL);
		if (FileUtils::IsDirectory(path))
		{
			m->path = path;
			return true;
		}

		// Search all possible installed paths.
		std::vector<InstallLocation*>::iterator i = installLocations.begin();
		while (i != installLocations.end())
		{
			InstallLocation* l = *i++;
			std::string p = FileUtils::Join(l->modules.c_str(), m->name.c_str(), NULL);
			std::string result = FileUtils::FindVersioned(p, m->requirement, m->version);
			if (!result.empty())
			{
				m->path = result;
				return true;
			}
		}

		return false; // We couldn't resolve this module!
	}

	bool FindRuntime(KComponent *m)
	{
		// Try to find the bundled version of this module.
		if (FileUtils::IsDirectory(this->bundledRuntimePath))
		{
			this->app->runtime->path = this->bundledRuntimePath;
			return true;
		}

		// Search all possible installed paths.
		std::vector<InstallLocation*>::iterator i = this->installLocations.begin();
		while (i != this->installLocations.end())
		{
			InstallLocation* l = *i++;
			std::string result = FileUtils::FindVersioned(l->runtime, m->requirement, m->version);

			// If we find a runtime in an installed location, make that the
			// default location for module installation.
			if (!result.empty())
			{
				this->activeRuntimeHome = l->runtimeHome;
				this->app->runtime->path = result;
				return true;
			}
		}

		return false; // We couldn't resolve this module!
	}

	std::string GetModuleList()
	{
		if (this->app->runtime == NULL || this->app->runtime->path.empty())
			throw std::string("Could not locate an appropriate runtime.");

		std::ostringstream moduleList;
		std::vector<KComponent*>::iterator i = this->app->modules.begin();
		while (i != this->app->modules.end())
		{
			KComponent *m = *i++;
			if (m->path.empty())
				throw std::string("Could not find module: ") + m->name;

			moduleList << m->path << ":";
		}

		return moduleList.str();
	}

	void RunInstaller(std::vector<KComponent*> missing)
    {
		// If we don't have an installer directory, just bail...
		const char* ci_path = this->installerPath.c_str();
		std::string installer = FileUtils::Join(ci_path, "installer", NULL);
		if (!FileUtils::IsDirectory(this->installerPath) || !FileUtils::IsFile(installer))
			throw std::string("Missing installer and application has additional modules that are needed.");

		std::vector<std::string> args;
		args.push_back("--apppath");
		args.push_back(app->path);

		if (!this->updateFile.empty())
		{
			args.push_back("--updatefile");
			args.push_back(this->updateFile);
		}

		std::vector<KComponent*>::iterator mi = missing.begin();
		while (mi != missing.end())
		{
			KComponent* mod = *mi++;
			std::string path =
				BootUtils::FindBundledModuleZip(mod->name, mod->version, this->applicationPath);
			if (path.empty())
			{
				path = mod->GetURL(this->app);
			}
			args.push_back(path);
		}

		kroll::FileUtils::RunAndWait(installer, args);
	}

};


int prepare_environment(int argc, const char* argv[])
{
	try
	{
		Boot boot = Boot(argv[0]);

		std::vector<KComponent*> missing = boot.FindModules();
		if (missing.size() > 0 || !boot.app->IsInstalled() || !boot.updateFile.empty())
		{
			boot.RunInstaller(missing);
			missing = boot.FindModules();
			boot.FindUpdate();
		}

		if (missing.size() > 0 || !boot.app->IsInstalled() || !boot.updateFile.empty())
		{
			// Don't throw an error here. The installer handles internal
			// errors and cancels on it's own. An error in the installer
			// preparation would have thrown an exception.
			return __LINE__;
		}

		// This is the signal that we are ready to boot
		setenv("KR_BOOT_READY", "1", 1);
		setenv("KR_HOME", boot.applicationPath.c_str(), 1);
		setenv("KR_RUNTIME", boot.app->runtime->path.c_str(), 1);
		setenv("KR_RUNTIME_HOME", boot.activeRuntimeHome.c_str(), 1);
		setenv("KR_APP_GUID", boot.app->guid.c_str(),1);
		setenv("KR_APP_ID", boot.app->id.c_str(), 1);

		std::string module_list = boot.GetModuleList();
		setenv("KR_MODULES", module_list.c_str(), 1);

		const char* prepath = getenv("LD_LIBRARY_PATH");
		std::ostringstream libraryPaths;
		libraryPaths << boot.app->runtime->path << ":" << module_list;
		if (prepath != NULL)
		 	libraryPaths << ":" << prepath;

		std::string libraryPathsStr = libraryPaths.str();
		setenv("LD_LIBRARY_PATH", libraryPathsStr.c_str(), 1);

		execv(argv[0], (char* const*) argv);
	}
	catch (std::string& e)
	{
		std::cout << e << std::endl;
	}
	return 0;
}

int start_host(int argc, const char* argv[])
{
	try
	{
		const char* rt_path = getenv("KR_RUNTIME");
		if (rt_path == NULL)
			return __LINE__;

		// now we need to load the host and get 'er booted
		std::string khost = FileUtils::Join(rt_path, "libkhost.so", NULL);
		if (!FileUtils::IsFile(khost))
			throw std::string("Couldn't find required file: ") + khost;

		void* khost_lib = dlopen(khost.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!khost_lib)
			throw std::string("Couldn't load host shared-object: ") + dlerror();

		Executor* executor = (Executor*) dlsym(khost_lib, "Execute");
		if (!executor)
			throw std::string("Could not find entry point for host.");

		int rc = executor(argc, argv);
		return rc;

	}
	catch (std::string& e)
	{
		std::cout << e << std::endl;
		return 1;
	}

}
#ifdef USE_BREAKPAD
#include "client/linux/handler/exception_handler.h"
#include "common/linux/http_upload.h"
#define CRASH_REPORT_OPT "--crash_report"
static std::string myself;
static google_breakpad::ExceptionHandler* breakpad;
bool breakpad_callback(
	const char* dump_path,
	const char* id,
	void* context,
	bool succeeded)
{
	if (succeeded)
	{
		std::string file = std::string(id) + ".dmp";
		file = FileUtils::Join(dump_path, file.c_str(), NULL);
		std::string exec_path = myself;
		exec_path.append(" ");
		exec_path.append(CRASH_REPORT_OPT);
		exec_path.append(" ");
		exec_path.append(file);
		exec_path.append("&");
		printf("Executing: %s\n", exec_path.c_str());
		fflush(stdout);
		system(exec_path.c_str());
	}
	return succeeded;
}

std::map<std::string, std::string> get_parameters(int argc, const char* argv[])
{
	std::map<std::string, std::string> params;
	for (int i = 2; argc > i+1; i++)
	{
		std::string key = argv[i];
		std::string value = argv[i+1];
		params[key] = value;
	}
	return params;
}

void send_crash_report(int argc, const char* argv[])
{
	std::string url = STRING(CRASH_REPORT_URL);
	const std::map<std::string, std::string> parameters = get_parameters(argc, argv);
	std::string upload_file = argv[2];
	std::string file_part_name = "dump";
	std::string proxy;
	std::string proxy_user_pwd;
	std::string response_body;
	std::string error_description;

	/*bool success =*/ google_breakpad::HTTPUpload::SendRequest(
		url,
		parameters,
		upload_file,
		file_part_name,
		proxy,
		proxy_user_pwd,
		&response_body,
		&error_description
	);

	/* Wait until there is a standard GUI for crash reports */
	//if (!success)
	//{
	//	GtkWidget* dialog = gtk_message_dialog_new(
	//		NULL,
	//		GTK_DIALOG_DESTROY_WITH_PARENT,
	//		GTK_MESSAGE_ERROR,
	//		GTK_BUTTONS_CLOSE,
	//		"Error uploading crash dump: %s",
	//		response_body.c_str());
	//	gtk_dialog_run(GTK_DIALOG(dialog));
	//	gtk_widget_destroy(dialog);
	//}
}
#endif


int main(int argc, const char* argv[])
{

#ifdef USE_BREAKPAD
	myself = argv[0];
	std::string dump_path = "/tmp";
	breakpad = new google_breakpad::ExceptionHandler(
		dump_path,
		NULL,
		breakpad_callback,
		NULL,
		true);

	if (argc > 2 && !strcmp(CRASH_REPORT_OPT, argv[1]))
	{
		send_crash_report(argc, argv);
	}
	else
#endif
	if (!getenv("KR_BOOT_READY"))
	{
		return prepare_environment(argc, argv);
	}
	else
	{
		unsetenv("KR_BOOT_READY");
		return start_host(argc, argv);
	}
}

