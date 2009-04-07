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
using kroll::FileUtils;

//
// these flags are compiled in to allow them
// to be tailed to the embedded environment
//
#ifndef _BOOT_UPDATESITE_ENVNAME
  #define _BOOT_UPDATESITE_ENVNAME UPDATESITE
#endif

#ifndef BOOT_UPDATESITE_ENVNAME
  #define BOOT_UPDATESITE_ENVNAME STRING(_BOOT_UPDATESITE_ENVNAME)
#endif

//
// these UUIDs should never change and uniquely identify a package type
//
#define DISTRIBUTION_URL "http://download.titaniumapp.com"
#define DISTRIBUTION_UUID "7F7FA377-E695-4280-9F1F-96126F3D2C2A"
#define RUNTIME_UUID "A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID "1ACE5D3A-2B52-43FB-A136-007BD166CFD0"

#define OS_NAME "linux"

#define MANIFEST_FILE "manifest"
#define MODULE_DIR "modules"
#define RUNTIME_DIR "runtime"

#ifdef OS_32
  #define OSTYPE "32bit"
#else
  #define OSTYPE "64bit"
#endif

typedef int Executor(int argc, const char **argv);
struct Module
{
	std::string name;
	std::string version;
	std::string path;
	std::string typeuuid;
	int op;
	std::vector<std::string> libs;

	Module(std::string key, std::string value)
		: name(key)
	{
		if (name == "runtime")
			this->typeuuid = RUNTIME_UUID;
		else
			this->typeuuid = MODULE_UUID;

		FileUtils::ExtractVersion(value, &this->op, this->version);
#ifdef DEBUG
		std::cout << "Component: " << this->name << " : " << this->version
		          << ", operation: " << this->op << std::endl;
#endif
	}
};

typedef struct InstallLocation
{
	std::string runtimeHome;
	std::string runtime;
	std::string modules;
} InstallLocation;

class Boot
{
	public:
	std::string app_path;
	std::string manifest_path;
	std::string app_name;
	std::string app_id;
	std::string guid;
	std::string installer_path;

	/* Default runtime base path or, if a runtime is found, that runtimeHomePath */
	std::string systemRuntimeHome;
	std::string userRuntimeHome;
	std::string activeRuntimeHome;

	/* Potential paths for installed modules and runtime */
	std::vector<InstallLocation*> installLocations;

	/* Paths for bundled modules and runtime */
	std::string bundledModulePath;
	std::string bundledRuntimePath;

	/* Modules requested by the manifest */
	std::vector<Module*> modules;
	Module *rt_module;

	std::map<std::string, void*> loaded_libraries;

	Boot() : rt_module(NULL)
	{
	}

	~Boot()
	{
		delete this->rt_module;

		std::vector<Module*>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			Module *m = *i++;
			delete m;
		}
		this->modules.clear();

		std::vector<InstallLocation*>::iterator li = this->installLocations.begin();
		while (li != this->installLocations.end())
		{
			InstallLocation* l = *li;
			li = this->installLocations.erase(li);
			delete l;
		}
	}

	void ParseManifest(const char* argv)
	{
		this->SetupPaths(argv);
		this->ReadApplicationManifest();
	}

	void SetupPaths(const char* argv)
	{
		char* buffer = (char*) alloca(sizeof(char) * PATH_MAX);
		char* real_path = realpath(argv, buffer);
		if (real_path == NULL)
		{
			this->app_path = FileUtils::GetApplicationDirectory();
		}
		else
		{
			struct stat statbuf;
			int result = stat(real_path, &statbuf);
			if (result == 0)
				this->app_path = std::string(dirname(real_path));
			else
				this->app_path = FileUtils::GetApplicationDirectory();
		}

		const char* capp_path = app_path.c_str();
		this->manifest_path = FileUtils::Join(capp_path, MANIFEST_FILE, NULL);
		if (!FileUtils::IsFile(this->manifest_path))
		{
			throw std::string("Could not find manifest!");
		}
		this->installer_path = FileUtils::Join(capp_path, "installer", NULL);

		std::string pname = PRODUCT_NAME;
		std::transform(pname.begin(), pname.end(), pname.begin(), tolower);

		// Kroll runtime and modules will located by searching the following paths in order:
		// 1. ~/.PRODUCT_NAME (eg. ~/.titanium)
		// 2. /opt/PRODUCT_NAME (default runtime base path for system-wide installation)
		// 3. /usr/local/lib/PRODUCT_NAME
		// 4. /usr/lib/PRODUCT_NAME
		std::string dotLocation = std::string(".") + pname;
		std::string homePath = getenv("HOME");
		this->userRuntimeHome = FileUtils::Join(homePath.c_str(), dotLocation.c_str(), NULL);
		this->AddInstallLocation(this->userRuntimeHome);

		std::string optLocation = std::string("/opt/") + pname;
		this->activeRuntimeHome = this->systemRuntimeHome = optLocation;
		this->AddInstallLocation(optLocation);

		this->AddInstallLocation(std::string("/usr/local/lib") + pname);
		this->AddInstallLocation(std::string("/usr/lib/") + pname);

		this->bundledModulePath = FileUtils::Join(capp_path, MODULE_DIR, NULL);
		this->bundledRuntimePath = FileUtils::Join(capp_path, RUNTIME_DIR, NULL);

		// The installer directory could previously contain bundled runtime / modules.
		// This location is no longer supported for Linux.
	}

	void AddInstallLocation(std::string path)
	{
		InstallLocation* location = new InstallLocation;
		location->runtimeHome = path;
		location->runtime = FileUtils::Join(path.c_str(), RUNTIME_DIR, "linux", NULL);
		location->modules = FileUtils::Join(path.c_str(), MODULE_DIR, "linux", NULL);
		this->installLocations.push_back(location);
	}

	void ReadApplicationManifest()
	{
		std::ifstream file(this->manifest_path.c_str());
		if (file.bad() || file.fail())
		{
			throw std::string("Application packaging error. The application manifest was not found in the correct location.");
		}

		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			line = FileUtils::Trim(line);

			size_t pos = line.find(":");
			if (pos == 0 || pos == line.length() - 1)
				continue;

			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1, line.length());
			key = FileUtils::Trim(key);
			value = FileUtils::Trim(value);

			if (key == "#appname")
			{
				this->app_name = value;
				continue;
			}
			else if (key == "#appid")
			{
				this->app_id = value;
				continue;
			}
			else if (key == "#guid")
			{
				this->guid = value;
				continue;
			}
			else if (key.c_str()[0] == '#')
			{
				continue;
			}
			else
			{
				Module* m = new Module(key, value);
				if (key == "runtime")
					this->rt_module = m;
				else
					this->modules.push_back(m);
			}
		}
	}

	std::vector<Module*> FindModules()
	{
		std::vector<Module*> unresolved;

		// Find the runtime module
		if (!this->FindRuntime(this->rt_module))
		{
			unresolved.push_back(this->rt_module);
		}

		// Find all regular modules
		std::vector<Module*>::iterator i = this->modules.begin();
		while (i != modules.end())
		{
			Module* m = *i++;
			if (!this->FindModule(m))
			{
				unresolved.push_back(m);
			}
		}

		if (unresolved.size() > 0)
		{
			std::vector<Module*>::iterator dmi = unresolved.begin();
			while (dmi != unresolved.end())
			{
				Module* m = *dmi++;
				std::cout << "Unresolved: " << m->name << std::endl;
			}
			std::cout << "---" << std::endl;
		}

		return unresolved;
	}

	bool FindModule(Module *m)
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
			std::string result = FileUtils::FindVersioned(p, m->op, m->version);
			if (!result.empty())
			{
				m->path = result;
				return true;
			}
		}

		return false; // We couldn't resolve this module!
	}

	bool FindRuntime(Module *m)
	{
		// Try to find the bundled version of this module.
		if (FileUtils::IsDirectory(this->bundledRuntimePath))
		{
			this->rt_module->path = this->bundledRuntimePath;
			return true;
		}

		// Search all possible installed paths.
		std::vector<InstallLocation*>::iterator i = this->installLocations.begin();
		while (i != this->installLocations.end())
		{
			InstallLocation* l = *i++;
			std::string result = FileUtils::FindVersioned(l->runtime, m->op, m->version);

			// If we find a runtime in an installed location, make that the
			// default location for module installation.
			if (!result.empty())
			{
				// Don't ever use the user's runtime home as the system runtime home
				if (l->runtimeHome != this->userRuntimeHome)
					this->systemRuntimeHome = l->runtimeHome;
				this->activeRuntimeHome = l->runtimeHome;

				this->rt_module->path = result;
				return true;
			}
		}

		return false; // We couldn't resolve this module!
	}

	std::string GetModuleList()
	{
		if (this->rt_module == NULL || this->rt_module->path.empty())
			throw std::string("Could not locate an appropriate runtime.");

		std::ostringstream moduleList;
		std::vector<Module*>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			Module *m = *i++;
			if (m->path.empty())
				throw std::string("Could not find module: ") + m->name;

			moduleList << m->path << ":";
		}

		return moduleList.str();
	}

	void RunAppInstaller(std::vector<Module*> missing)
    {

		// If we don't have an installer directory, just bail...
		const char* ci_path = this->installer_path.c_str();
		std::string installer = FileUtils::Join(ci_path, "installer", NULL);
		printf("%s\n", installer.c_str());
		if (!FileUtils::IsDirectory(this->installer_path) || !FileUtils::IsFile(installer))
		{
			throw std::string("Missing installer and application has additional modules that are needed.");
		}

		std::string temp_dir = FileUtils::GetTempDirectory();
		std::string mid = PlatformUtils::GetMachineId();
		std::string os = OS_NAME;
		std::string osver = FileUtils::GetOSVersion();
		std::string osarch = FileUtils::GetOSArchitecture();

		std::string qs;
		qs += "?os=" + FileUtils::EncodeURIComponent(os);
		qs += "&osver=" + FileUtils::EncodeURIComponent(osver);
		qs += "&tiver=" + FileUtils::EncodeURIComponent(STRING(_PRODUCT_VERSION));
		qs += "&mid=" + FileUtils::EncodeURIComponent(mid);
		qs += "&aid=" + FileUtils::EncodeURIComponent(this->app_id);
		qs += "&guid=" + FileUtils::EncodeURIComponent(guid);
		qs += "&ostype=" + FileUtils::EncodeURIComponent(OSTYPE);
		qs += "&osarch=" + FileUtils::EncodeURIComponent(osarch);

		// Figure out the update site URL
		std::string url;
		char* env_site = getenv(BOOT_UPDATESITE_ENVNAME);
		if (env_site != NULL)
			url = std::string(env_site);
		else
			url = std::string(DISTRIBUTION_URL);

		if (url.empty())
		{
			kroll::FileUtils::DeleteDirectory(temp_dir); // Clean up
			throw std::string("Modules missing and could not find a download site.");
		}

		//I18N: localize here
		std::vector<std::string> args;
		args.push_back("--initial"); // Ask the user for the install type
		args.push_back(this->systemRuntimeHome); // Default system runtime location
		args.push_back(this->userRuntimeHome); // Default user runtime location
		args.push_back(this->app_name); // appname
		args.push_back(temp_dir); // temp dir

		std::vector<Module*>::iterator mi = missing.begin();
		while (mi != missing.end())
		{
			Module* mod = *mi++;
			std::string u(url);
			u.append(qs);
			u.append("&name=");
			u.append(mod->name);
			u.append("&version=");
			u.append(mod->version);
			u.append("&uuid=");
			u.append(mod->typeuuid);
			args.push_back(u);
		}

		kroll::FileUtils::RunAndWait(installer, args);
		kroll::FileUtils::DeleteDirectory(temp_dir);
	}

};


int prepare_environment(int argc, const char* argv[])
{
	try
	{
		Boot boot = Boot();

		boot.ParseManifest(argv[0]);
		std::vector<Module*> missing = boot.FindModules();
		if (missing.size() > 0)
		{
			boot.RunAppInstaller(missing);
			missing = boot.FindModules();
		}

		if (missing.size() > 0)
		{
			// Don't throw an error here, because the module installer
			// would have thrown an exception on an error, so the user
			// must have cancelled.
			return __LINE__;
		}

		// GetModulList also ensures all modules have been found
		std::string module_list = boot.GetModuleList();

		// This is the signal that we are ready to boot
		setenv("KR_BOOT_READY", "1", 1);
		setenv("KR_HOME", boot.app_path.c_str(), 1);
		setenv("KR_RUNTIME", boot.rt_module->path.c_str(), 1);
		setenv("KR_MODULES", module_list.c_str(), 1);
		setenv("KR_RUNTIME_HOME", boot.activeRuntimeHome.c_str(), 1);
		setenv("KR_APP_GUID", boot.guid.c_str(),1);
		setenv("KR_APP_ID", boot.app_id.c_str(), 1);

		const char* prepath = getenv("LD_LIBRARY_PATH");
		if (prepath == NULL) prepath = "";

		std::ostringstream ld_library_path;
		ld_library_path << boot.rt_module->path << ":"
		                 << module_list << ":" << prepath;
		setenv("LD_LIBRARY_PATH", ld_library_path.str().c_str(), 1);
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
		{
			throw std::string("Couldn't find required file: ") + khost;
		}

		void* khost_lib = dlopen(khost.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!khost_lib)
		{
			throw std::string("Couldn't load host shared-object: ") + dlerror();
		}
		Executor* executor = (Executor*) dlsym(khost_lib, "Execute");
		if (!executor)
		{
		throw std::string("Could not find entry point for host.");
	}

	int rc = executor(argc, argv);
#ifdef DEBUG
		std::cout << "return code: " << rc << std::endl;
#endif
		return rc;
	}
	catch (std::string& e)
	{
		std::cout << e << std::endl;
	}
	return 0;

}

int main(int argc, const char* argv[])
{
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

