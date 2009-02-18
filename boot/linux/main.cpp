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


#include "file_utils.h"

// ensure that Kroll API is never included to create
// an artificial dependency on kroll shared library
#ifdef _KROLL_H_
#error You should not have included the kroll api!
#endif

using namespace kroll;

//
// these flags are compiled in to allow them
// to be tailed to the embedded environment
//
#ifndef _BOOT_UPDATESITE_ENVNAME
  #define _BOOT_UPDATESITE_ENVNAME UPDATESITE
#endif

#ifndef _BOOT_UPDATESITE_URL
  #error "Define _BOOT_UPDATESITE_URL"
#endif

#ifndef BOOT_UPDATESITE_ENVNAME
  #define BOOT_UPDATESITE_ENVNAME STRING(_BOOT_UPDATESITE_ENVNAME)
#endif

#ifndef BOOT_UPDATESITE_URL
  #define BOOT_UPDATESITE_URL STRING(_BOOT_UPDATESITE_URL)
#endif

#define OS_NAME "linux"

#define MANIFEST_FILE "manifest"
#define MODULE_DIR "modules"
#define RUNTIME_DIR "runtime"

typedef int Executor(int argc, const char **argv);
struct Module
{
	std::string name;
	std::string version;
	std::string path;
	int op;
	std::vector<std::string> libs;

	Module(std::string key, std::string value)
		: name(key)
	{
		FileUtils::ExtractVersion(value, &this->op, this->version);
#ifdef DEBUG
		std::cout << "Component: " << this->name << ":" << this->version
		          << ", operation: " << this->op << std::endl;
#endif
	}

	void Prep()
	{
		//this->ReadManifest();
		//this->LoadLibraries();
	}

	void ReadManifest()
	{
		std::string fn = FileUtils::Join(this->path.c_str(), "manifest", NULL);
		if (!FileUtils::IsFile(fn))
			return;

		std::ifstream file(fn.c_str());
		if (file.bad() || file.fail())
			throw std::string("Could not read module manifest: ") + fn;

		while (!file.eof())
		{
			std::string line;
			std::getline(file,line);
			if (line.empty() || line.find(" ") == 0 || line.find("#") == 0)
				continue;

			std::string libname(FileUtils::Trim(line));
			if (libname.empty())
				continue;

			std::string path = FileUtils::Join(this->path.c_str(), libname.c_str(), NULL);
			this->libs.push_back(path);
		}
	}

	void LoadLibraries()
	{
		std::vector<std::string>::iterator i = this->libs.begin();
		while (i != this->libs.end())
		{
			std::string lib = *i++;
#ifdef DEBUG
			std::cout << "Attempting to load: " << lib << std::endl;
#endif
			void* r = dlopen(lib.c_str(), RTLD_LAZY | RTLD_GLOBAL);
			if (r == NULL)
				throw std::string("Couldn't load required library: ") + lib;
		}
	}
};

class Boot
{
	public:
	std::string app_path;
	std::string manifest_path;
	std::string rt_path;
	std::string app_name;
	std::string app_id;
	std::string installer_path;

	/* Potential paths for installed modules and runtime */
	std::vector<std::string> module_paths;
	std::vector<std::string> rt_paths;

	/* Paths for bundled modules and runtime */
	std::string bundled_module_path;
	std::string bundled_rt_path;
	std::string bundled_installer_module_path;
	std::string bundled_installer_rt_path;

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
	}

	void ParseManifest(const char* argv0)
	{
		this->SetupPaths(argv0);
		this->ReadApplicationManifest();
	}

	void SetupPaths(const char* argv0)
	{
		char* buffer = (char*) alloca(sizeof(char) * PATH_MAX);
		char* real_path = realpath(argv0, buffer);
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
			throw std::string("Could not find manifest!");

		this->rt_path = FileUtils::GetRuntimeBaseDirectory();
		if (!FileUtils::IsDirectory(this->rt_path))
		{
			std::cerr << "Could not find runtime directory ("
			          << rt_path << "), but charging ahead anyhow."
			          << std::endl;
		}
		else
		{
			// Later this will be a list of usual locations that
			// modules might be installed. For now it is the default
			// runtime location.
			const char* crt_path = this->rt_path.c_str();
			std::string rt_module_path =
				 FileUtils::Join(crt_path, MODULE_DIR, NULL);
			this->module_paths.push_back(rt_module_path);

			std::string rt_rt_path =
				 FileUtils::Join(crt_path, RUNTIME_DIR, "linux", NULL);
			this->rt_paths.push_back(rt_rt_path);

		}

		this->bundled_module_path = FileUtils::Join(capp_path, MODULE_DIR, NULL);
		this->bundled_rt_path = FileUtils::Join(capp_path, RUNTIME_DIR, NULL);

		// The installer directory contains the package installer and bundled
		// runtime / modules may also reside there. These locations have a lower
		// priority than all other locations.
		this->installer_path = kroll::FileUtils::Join(capp_path, "installer", NULL);
		this->bundled_installer_module_path = FileUtils::Join(installer_path.c_str(), MODULE_DIR, NULL);
		this->bundled_installer_rt_path = FileUtils::Join(installer_path.c_str(), RUNTIME_DIR, NULL);
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
		this->rt_module->path = this->FindRuntime(this->rt_module);
		if (this->rt_module->path.empty())
		{
			unresolved.push_back(this->rt_module);
		}

		// Find all regular modules
		std::vector<Module*>::iterator i = this->modules.begin();
		while (i != modules.end())
		{
			Module* m = *i++;
			m->path = this->FindModule(m);
			if (m->path.empty())
			{
				unresolved.push_back(m);
			}
		}

		return unresolved;
	}

	std::string FindModule(Module *m)
	{
		// Try to find the bundled version of this module.
		std::string path = FileUtils::Join(this->bundled_module_path.c_str(), m->name.c_str(), NULL);
		if (FileUtils::IsDirectory(path))
			return path;

		// Search all possible installed paths.
		std::vector<std::string>::iterator i = module_paths.begin();
		while (i != module_paths.end())
		{
			std::string p = *i++;
			p = FileUtils::Join(p.c_str(), m->name.c_str(), NULL);
			std::string result = FileUtils::FindVersioned(p, m->op, m->version);
			if (!result.empty())
				return result;
		}

		// Try to find this module bundled with the installer
		path = FileUtils::Join(this->bundled_installer_module_path.c_str(), m->name.c_str(), NULL);
		if (FileUtils::IsDirectory(path))
			return path;

		return std::string(); // We couldn't resolve this module!
	}

	std::string FindRuntime(Module *m)
	{
		// Try to find the bundled version of this module.
		if (FileUtils::IsDirectory(this->bundled_rt_path))
			return this->bundled_rt_path;

		// Search all possible installed paths.
		std::vector<std::string>::iterator i = this->rt_paths.begin();
		while (i != this->rt_paths.end())
		{
			std::string p = *i++;
			std::string result = FileUtils::FindVersioned(p, m->op, m->version);
			if (!result.empty())
				return result;
		}

		// Try to find this module bundled with the installer
		if (FileUtils::IsDirectory(this->bundled_installer_module_path))
			return this->bundled_installer_module_path;

		return std::string(); // We couldn't resolve this module!
	}

	std::string PrepModules()
	{
		if (this->rt_module == NULL || this->rt_module->path.empty())
			throw std::string("Could not locate an appropriate runtime.");

		this->rt_module->Prep();

		std::ostringstream moduleList;
		std::vector<Module*>::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			Module *m = *i++;
			if (m->path.empty())
				throw std::string("Could not find module: ") + m->name;

			m->Prep();
			moduleList << m->path << ":";
		}

		return moduleList.str();
	}

	void RunAppInstaller(std::vector<Module*> missing)
    {
#ifdef DEBUG
		std::vector<Module*>::iterator dmi = missing.begin();
		while (dmi != missing.end())
		{
			Module* m = *dmi++;
			std::cout << "Missing: " << m->name << std::endl;
		}
#endif

		// If we don't have an installer directory, just bail...
		const char* ci_path = this->installer_path.c_str();
		std::string installer = kroll::FileUtils::Join(ci_path, "installer", NULL);
		if (!FileUtils::IsDirectory(this->installer_path) || !FileUtils::IsFile(installer))
		{
			throw std::string("Missing installer and application has additional modules that are needed.");
		}

		std::string temp_dir = kroll::FileUtils::GetTempDirectory();
		std::string sid = kroll::FileUtils::GetMachineId();
		std::string os = OS_NAME;
		std::string qs("?os="+os+"&sid="+sid+"&aid="+this->app_id);

		// Install to default runtime directory. At some point
		// net_installer will decide where to install (for Loonix)
		std::string install_to = this->rt_path;
		kroll::FileUtils::CreateDirectory(install_to);

		// Figure out the update site URL
		std::string url;
		if (strlen(BOOT_UPDATESITE_URL))
			url = std::string(BOOT_UPDATESITE_URL);
		char* env_site = getenv(BOOT_UPDATESITE_ENVNAME);
		if (env_site != NULL)
			url = std::string(env_site);

		if (url.empty())
		{
			kroll::FileUtils::DeleteDirectory(temp_dir); // Clean up
			throw std::string("Modules missing and could not find a download site.");
		}

		//I18N: localize here
		std::vector<std::string> args;
		args.push_back(this->app_name); // appname
		args.push_back("Additional application files required"); // title
		args.push_back("There are additional application files that are required for this application. These will be downloaded from the network. Please press Continue to download these files now to complete the installation of the application."); // intro
		args.push_back(temp_dir); // temp dir
		args.push_back(install_to); // where to install
		args.push_back("unused"); // unused unzip var

		std::vector<Module*>::iterator mi = missing.begin();
		while (mi != missing.end())
		{
			Module* mod = *mi++;
#ifdef DEBUG
			std::cout << "Trying to install: " << mod->name << "/" << mod->version <<std::endl;
#endif
			std::string mod_url = url + "/" + mod->name + ".zip" + qs;
			args.push_back(mod_url);
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

		// PropModules also ensure all modules have been found
		std::string module_list = boot.PrepModules();

		// This is the signal that we are ready to boot
		setenv("KR_BOOT_READY", "1", 1);
		setenv("KR_HOME", boot.app_path.c_str(), 1);
		setenv("KR_RUNTIME", boot.rt_module->path.c_str(), 1);
		setenv("KR_MODULES", module_list.c_str(), 1);
		setenv("KR_RUNTIME_HOME", boot.rt_path.c_str(), 1);

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

