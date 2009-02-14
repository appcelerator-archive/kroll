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
		this->ReadManifest();
		this->LoadLibraries();
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

	/* Potential paths for installed modules and runtime */
	std::vector<std::string> module_paths;
	std::vector<std::string> rt_paths;

	/* Path for bundled modules and runtime */
	std::string bundled_module_path;
	std::string bundled_rt_path;

	/* Modules requested by the manifest */
	std::vector<Module*> modules;
	Module *rt_module;

	std::map<std::string, void*> loaded_libraries;

	Boot() :
		app_path(""),
		manifest_path(""),
		rt_path(""),
		app_name(""),
		app_id(""),
		rt_module(NULL)
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

	void ParseManifest()
	{
		this->SetupPaths();
		this->ReadApplicationManifest();
	}

	void SetupPaths()
	{
		this->app_path = FileUtils::GetApplicationDirectory();
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

		const char* crt_path = this->rt_path.c_str();
		this->bundled_module_path = FileUtils::Join(capp_path, MODULE_DIR, NULL);
		this->bundled_rt_path = FileUtils::Join(capp_path, RUNTIME_DIR, NULL);

		// Later this will be a list of usual locations that
		// modules might be installed. For now it is the default
		// runtime location.
		std::string rt_module_path =
			 FileUtils::Join(crt_path, MODULE_DIR, NULL);
		this->module_paths.push_back(rt_module_path);

		std::string rt_rt_path =
			 FileUtils::Join(crt_path, RUNTIME_DIR, "linux", NULL);
		this->rt_paths.push_back(rt_module_path);
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
		if (!this->FindRuntime(this->rt_module))
			unresolved.push_back(this->rt_module);

		// Find all regular modules
		std::vector<Module*>::iterator i = this->modules.begin();
		while (i != modules.end())
		{
			Module* m = *i++;
			if (!this->FindRegularModule(m))
			{
				unresolved.push_back(m);
			}
		}

		return unresolved;
	}

	bool FindRegularModule(Module *m)
	{
		return this->FindModule(m, this->bundled_module_path, this->module_paths);
	}

	bool FindRuntime(Module *m)
	{
		return this->FindModule(m, this->bundled_rt_path, this->rt_paths);
	}

	bool FindModule(
		Module* module,
		std::string& bundled_path,
		std::vector<std::string>& installed_paths)
	{
		// Try to find the bundled version of this module.
		std::string path = FileUtils::Join(bundled_path.c_str(), module->name.c_str(), NULL);
		if (FileUtils::IsDirectory(path))
		{
			module->path = path;
			return true;
		}

		// Search all possible installed paths.
		std::vector<std::string>::iterator i = installed_paths.begin();
		while (i != installed_paths.end())
		{
			std::string result = FileUtils::FindVersioned(*i, module->op, module->version);
			if (!result.empty())
			{
				module->path = path;
				return true;
			}
		}

		return false; // We couldn't resolve this module!
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
			moduleList << m->path << ";";
		}

		return moduleList.str();
	}

	//bool RunAppInstallerIfNeeded(std::string &homedir,
	//					 std::string &manifest,
	//					 std::vector< std::pair< std::pair<std::string,std::string>,bool> > &modules,
	//					 std::vector<std::string> &moduleDirs,
	//					 std::string &appname,
	//					 std::string &appid,
	//					 std::string &runtimeOverride)
    //{
	//std::string runtimePath = this->rt_path;

	//bool result = true;
	//std::vector< std::pair<std::string,std::string> > missing;
	//std::vector< std::pair< std::pair<std::string,std::string>, bool> >::iterator i = modules.begin();
	//while(i!=modules.end())
	//{
	//	std::pair< std::pair<std::string,std::string>,bool> p = (*i++);
	//	if (!p.second)
	//	{
	//		missing.push_back(p.first);
    //#ifdef DEBUG
	//		std::cout << "missing module: " << p.first.first << "/" << p.first.second <<std::endl;
    //#endif
	//	}
	//}
	//// this is where kroll should be installed
	//std::string runtimeBase = kroll::FileUtils::GetRuntimeBaseDirectory();

	//if (missing.size()>0)
	//{
	//	// if we don't have an installer directory, just bail...
	//	std::string installerDir = kroll::FileUtils::Join(homedir.c_str(),"installer",NULL);

	//	std::string sourceTemp = kroll::FileUtils::GetTempDirectory();
	//	std::vector<std::string> args;
	//	// appname
	//	args.push_back(appname);
	//	// title
	//	//I18N: localize these
	//	args.push_back("Additional application files required");
	//	// message
	//	//I18N: localize these
	//	args.push_back("There are additional application files that are required for this application. These will be downloaded from the network. Please press Continue to download these files now to complete the installation of the application.");
	//	// extract directory
	//	args.push_back(sourceTemp);
	//	// runtime base
	//	args.push_back(runtimeBase);
	//	// in Win32 installer, we push the path to this process so he can
	//	// invoke back on us to do the unzip
	//	args.push_back(GetExecutablePath());

	//	// make sure we create our runtime directory
	//	kroll::FileUtils::CreateDirectory(runtimeBase);

	//	char updatesite[MAX_PATH];
	//	int size = GetEnvironmentVariable(BOOT_UPDATESITE_ENVNAME,(char*)&updatesite,MAX_PATH);
	//	updatesite[size]='\0';
	//	std::string url;
	//	if (size == 0)
	//	{
	//		const char *us = BOOT_UPDATESITE_URL;
	//		if (strlen(us)>0)
	//		{
	//			url = std::string(us);
	//		}
	//	}
	//	else
	//	{
	//		url = std::string(updatesite);
	//	}
	//	
	//	if (!url.empty())
	//	{
	//		std::string sid = kroll::FileUtils::GetMachineId();
	//		std::string os = OS_NAME;
	//		std::string qs("?os="+os+"&sid="+sid+"&aid="+appid);
	//		std::vector< std::pair<std::string,std::string> >::iterator iter = missing.begin();
	//		int missingCount = 0;
	//		while (iter!=missing.end())
	//		{
	//			std::pair<std::string,std::string> p = (*iter++);
	//			std::string name;
	//			std::string path;
	//			bool found = false;
	//			if (p.first == "runtime")
	//			{
	//				name = "runtime-" + os + "-" + p.second;
	//				// see if we have a private runtime installed and we can link to that
	//				path = kroll::FileUtils::Join(installerDir.c_str(),"runtime",NULL);
	//				if (kroll::FileUtils::IsDirectory(path))
	//				{
	//						found = true;
	//						runtimePath = path;
	//				}
	//			}
	//			else
	//			{
	//				name = "module-" + p.first + "-" + p.second;
	//				// see if we have a private module installed and we can link to that
	//				path = kroll::FileUtils::Join(installerDir.c_str(),"modules",p.first.c_str(),NULL);
	//				if (kroll::FileUtils::IsDirectory(path))
	//				{
	//					found = true;
	//				}
	//			}
	//			if (found)
	//			{
	//				moduleDirs.push_back(path);
	//			}
	//			else
	//			{
	//				std::string u(url);
	//				u+="/";
	//				u+=name;
	//				u+=".zip";
	//				u+=qs;
	//				args.push_back(u);
	//				missingCount++;
	//			}
	//		}

	//		// we have to check again in case the private module/runtime was
	//		// resolved inside the application folder
	//		if (missingCount>0)
	//		{
	//			// run the installer app which will fetch remote modules/runtime for us
	//			std::string exec = kroll::FileUtils::Join(installerDir.c_str(),"Installer.exe",NULL);

	//			// paranoia check
	//			if (kroll::FileUtils::IsFile(exec))
	//			{
	//				// run and wait for it to exit..
	//				kroll::FileUtils::RunAndWait(exec,args);

	//				modules.clear();
	//				moduleDirs.clear();
	//				bool success = kroll::FileUtils::ReadManifest(manifest,runtimePath,modules,moduleDirs,appname,appid,runtimeOverride);
	//				if (!success || modules.size()!=moduleDirs.size())
	//				{
	//					// must have failed
	//					// no need to error has installer probably was cancelled
	//					result = false;
	//				}
	//			}
	//			else
	//			{
	//				// something crazy happened
	//				result = false;
	//				KR_FATAL_ERROR("Missing installer and application has additional modules that are needed.");
	//			}
	//		}
	//	}
	//	else
	//	{
	//		result = false;
	//		KR_FATAL_ERROR("Missing installer and application has additional modules that are needed. Not updatesite has been configured.");
	//	}
	//	
	//	// unlink the temporary directory
	//	kroll::FileUtils::DeleteDirectory(sourceTemp);
	//}
	//return result;
};

int main(int argc, const char* argv[])
{

	Boot boot = Boot();

	try
	{
		boot.ParseManifest();

		std::vector<Module*> missing = boot.FindModules();

	// run the app installer if any missing modules/runtime or
	// version specs not met
	//if (!RunAppInstallerIfNeeded(homedir,runtimePath,manifest,modules,moduleDirs,appname,appid,runtimeOverride))
	//{
	//	return __LINE__;
	//}

		// PropModules also ensure all modules have been found
		std::string module_list = boot.PrepModules();

		// NOTE: we use putenv explicitly because we use getenv in host
		// TODO: We need to clean up this memory
		std::ostringstream krhome;
		krhome << "KR_HOME=" << boot.app_path;
		putenv(strdup(krhome.str().c_str()));

		std::ostringstream krruntime;
		krruntime << "KR_RUNTIME=" << boot.rt_module->path;
		putenv(strdup(krruntime.str().c_str()));

		std::ostringstream krmodules;
		krmodules << "KR_MODULES=" << module_list;
		putenv(strdup(krmodules.str().c_str()));

		std::ostringstream krruntimehome;
		krruntimehome << "KR_RUNTIME_HOME=" << boot.rt_path;
		putenv(strdup(krruntimehome.str().c_str()));

		// now we need to load the host and get 'er booted
		std::string khost = FileUtils::Join(boot.rt_module->path.c_str(), "khost.so", NULL);
		if (!FileUtils::IsFile(khost))
		{
			throw std::string("Couldn't find required file: ") + khost;
		}

		void* khost_lib = dlopen(khost.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!khost_lib)
		{
			throw std::string("Couldn't load file: ") + khost;
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
		// Handle exceptions here
	}
}


