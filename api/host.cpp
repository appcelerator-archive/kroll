/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#endif
#if defined(OS_WIN32)
#include "base.h"
#include <windows.h>
#else
# include <dirent.h>
#endif
#include <cerrno>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>

#include "kroll.h"
#include <Poco/DirectoryIterator.h>
#include <Poco/File.h>
#include <Poco/Environment.h>

using Poco::Environment;

namespace kroll
{
	SharedPtr<Host> Host::instance_;

	Host::Host(int argc, const char *argv[]) : debug(false)
	{
		instance_ = this;

		std::string kr_home_str = "KR_HOME";
		std::string kr_runtime_str = "KR_RUNTIME";
		std::string kr_modules_str = "KR_MODULES";

		if (!Environment::has(kr_home_str))
		{
			std::cerr << kr_home_str << " not defined, aborting." << std::endl;
			exit(1);
		}

		if (!Environment::has(kr_runtime_str))
		{
			std::cerr << kr_runtime_str << " not defined, aborting." << std::endl;
			exit(1);
		}

		if (!Environment::has(kr_modules_str))
		{
			std::cerr << kr_modules_str << " not defined, aborting." << std::endl;
			exit(1);
		}

		std::string ti_home = Environment::get(kr_home_str);
		std::string ti_runtime = Environment::get(kr_runtime_str);
		std::string paths = Environment::get(kr_modules_str);
		PRINTD(">>> " << kr_home_str << "=" << ti_home);
		PRINTD(">>> " << kr_runtime_str << "=" << ti_runtime);
		PRINTD(">>> " << kr_modules_str << "=" << paths);

		FileUtils::Tokenize(paths, this->module_paths, KR_LIB_SEP);
		// check to see if we have to install the app
		std::string home = SetupAppInstallerIfRequired(ti_home);

		// check to see if we have a different starting point
		std::string start_page = SetupStartPageOverrideIfRequired(argc,argv);
		if (!start_page.empty())
		{
			// will return non-empty if have a non-default home
			home = start_page;
		}

		// change the KR_HOME environment if changed
		if (home != ti_home)
		{
			Environment::set(kr_home_str, home);
		}


		this->running = false;
		this->exitCode = 0;

		this->appDirectory = std::string(home);
		this->runtimeDirectory = std::string(ti_runtime);
		this->global_object = new StaticBoundObject();

		// link the name of our global variable to ourself so
		//  we can reference from global scope directly to get it
		this->global_object->SetObject(GLOBAL_NS_VARNAME, this->global_object);

		this->autoScan = false;
		this->runUILoop = true;

		// Sometimes libraries parsing argc and argv will
		// modify them, so we want to keep our own copy here
		for (int i = 0; i < argc; i++)
		{
			this->args.push_back(std::string(argv[i]));
			PRINTD("ARGUMENT[" << i << "] => " << argv[i]);

			if (strcmp(argv[i],"--debug")==0)
			{
				std::cout << "DEBUGGING DETECTED!" << std::endl;
				this->debug = true;
			}
		}
	}

	Host::~Host()
	{
	}

	std::string Host::SetupStartPageOverrideIfRequired(int argc, const char**argv)
	{
		if (argc > 1)
		{
			for (int c=1;c<argc;c++)
			{
				std::string arg(argv[c]);
				size_t i = arg.find("--start=");
				if (i!=std::string::npos)
				{
					size_t i = arg.find("=");
					return arg.substr(i+1);
				}
			}
		}
		return std::string();
	}

	std::string Host::FindAppInstaller(std::string home)
	{
		std::string appinstaller = FileUtils::Join(home.c_str(), "appinstaller", NULL);
		if (FileUtils::IsDirectory(appinstaller))
		{
			PRINTD("Found app installer at: " << appinstaller);
			return appinstaller.c_str();
		}

		std::string ename = "KR_RUNTIME";
		if (Environment::has(ename))
		{
			std::string runtime = Environment::get(ename);
			appinstaller = FileUtils::Join(runtime.c_str(), "appinstaller", NULL);
			if (FileUtils::IsDirectory(appinstaller))
			{
				PRINTD("Found app installer at: " << appinstaller);
				return appinstaller.c_str();
			}
		}

		PRINTD("Couldn't find app installer");
		return std::string();
	}

	std::string Host::SetupAppInstallerIfRequired(std::string home)
	{
		std::string marker = FileUtils::Join(home.c_str(), ".installed", NULL);
		if (!FileUtils::IsFile(marker))
		{
			// not yet installed, look for the app installer
			std::string ai = FindAppInstaller(home);
			if (!ai.empty())
			{
				// make the app installer our new home so that the
				// app installer will run and set the environment var
				// to the special setting with the old home
				std::string ename = "KR_APP_INSTALL_FROM";
				Environment::set(ename,  home);
				return FileUtils::Trim(ai);
			} 
			PRINTD("Application needs installation but no app installer found");
		}
		return home;
	}

	bool Host::IsDebugMode()
	{
		return this->debug;
	}

	const int Host::GetCommandLineArgCount()
	{
		return this->args.size();
	}

	const char* Host::GetCommandLineArg(int index)
	{
		if ((int) this->args.size() > index)
		{
			return this->args.at(index).c_str();
		}
		else
		{
			return NULL;
		}
	}

	void Host::AddModuleProvider(ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);
		module_providers.push_back(provider);

		if (autoScan)
		{
			this->ScanInvalidModuleFiles();
		}

	}

	ModuleProvider* Host::FindModuleProvider(std::string& filename)
	{
		ScopedLock lock(&moduleMutex);

		std::vector<ModuleProvider*>::iterator iter;
		for (iter = module_providers.begin();
		     iter != module_providers.end();
		     iter++)
		{
			ModuleProvider *provider = (*iter);
			if (provider != NULL && provider->IsModule(filename)) {
				return provider;
			}
		}
		return NULL;
	}

	void Host::RemoveModuleProvider(ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);

		std::vector<ModuleProvider*>::iterator iter =
		    std::find(module_providers.begin(),
		              module_providers.end(), provider);
		if (iter != module_providers.end()) {
			module_providers.erase(iter);
		}

	}
	void Host::UnloadModuleProviders()
	{
		while (module_providers.size() > 0)
		{
			ModuleProvider* provider = module_providers.at(0);
			this->RemoveModuleProvider(provider);
		}
	}

	bool Host::IsModule(std::string& filename)
	{
		std::string suffix = this->GetModuleSuffix();
		bool isModule = (filename.length() > suffix.length() && filename.substr(
				filename.length() - suffix.length()) == suffix);

		return isModule;
	}

	void Host::CopyModuleAppResources(std::string& modulePath)
	{
		std::cout << "CopyModuleAppResources: " << modulePath << std::endl;

		std::string appDir = FileUtils::GetApplicationDirectory();

		try {
			Poco::Path moduleDir(modulePath);
			moduleDir = moduleDir.parent();
			std::string mds(moduleDir.toString());

			const char* platform = this->GetPlatform();
			std::string resources_dir = FileUtils::Join(mds.c_str(), "AppResources", NULL);
			std::string plt_resources_dir = FileUtils::Join(resources_dir.c_str(), platform, NULL);
			std::string all_resources_dir = FileUtils::Join(resources_dir.c_str(), "all", NULL);
			Poco::File platformAppResourcesDir(plt_resources_dir);
			Poco::File allAppResourcesDir(all_resources_dir);

			if (platformAppResourcesDir.exists()
				&& platformAppResourcesDir.isDirectory()) {

				std::vector<Poco::File> files;
				platformAppResourcesDir.list(files);
				for (size_t i = 0; i < files.size(); i++) {
					std::cout << "Copying " << files.at(i).path() << " to " << appDir << std::endl;
					files.at(i).copyTo(appDir);
				}
			}

			if (allAppResourcesDir.exists()
				&& allAppResourcesDir.isDirectory())
			{
				std::vector<Poco::File> files;
				allAppResourcesDir.list(files);
				for (size_t i = 0; i < files.size(); i++) {
					std::cout << "Copying " << files.at(i).path() << " to " << appDir << std::endl;
					files.at(i).copyTo(appDir);
				}
			}
		} catch (Poco::Exception &exc) {
			// Handle..
		}
	}

	SharedPtr<Module> Host::LoadModule(std::string& path, ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);

		SharedPtr<Module> module = NULL;
		try
		{
			this->CopyModuleAppResources(path);
			module = provider->CreateModule(path);
			module->SetProvider(provider); // set the provider

			std::cout << "Loaded " << module->GetName()
			          << " (" << path << ")" <<std::endl;

			// Call module Load lifecycle event
			module->Initialize();

			// Store module
			this->modules[path] = module;
			this->loaded_modules.push_back(module);
		}
		catch (kroll::ValueException& e)
		{
			SharedString s = e.GetValue()->DisplayString();
			std::cerr << "Error generated loading module ("
			          << path << "): " << *s << std::endl;
		}
		catch(std::exception &e)
		{
			std::cerr << "Error generated loading module (" << path <<"): " << e.what()<< std::endl;
		}
		catch(...)
		{
			std::cerr << "Error generated loading module: " << path << std::endl;
		}

		return module;
	}

	void Host::UnloadModules()
	{
		ScopedLock lock(&moduleMutex);

		// Stop all modules
		while (this->loaded_modules.size() > 0)
		{
			this->UnregisterModule(this->loaded_modules.at(0));
		}

	}

	void Host::LoadModules()
	{
		ScopedLock lock(&moduleMutex);

		/* Scan module paths for modules which can be
		 * loaded by the basic shared-object provider */
		std::vector<std::string>::iterator iter;
		iter = this->module_paths.begin();
		while (iter != this->module_paths.end())
		{
			this->FindBasicModules((*iter++));
		}

		/* All modules are now loaded,
		 * so start them all */
		this->StartModules(this->loaded_modules);

		/* Try to load files that weren't modules
		 * using newly available module providers */
		this->ScanInvalidModuleFiles();

		/* From now on, adding a module provider will trigger
		 * a rescan of all invalid module files */
		this->autoScan = true;
	}

	void Host::FindBasicModules(std::string& dir)
	{
		ScopedLock lock(&moduleMutex);

		Poco::DirectoryIterator iter = Poco::DirectoryIterator(dir);
		Poco::DirectoryIterator end;
		while (iter != end)
		{
			Poco::File f = *iter;
			if (!f.isDirectory() && !f.isHidden())
			{
				std::string fpath = iter.path().absolute().toString();
				if (IsModule(fpath))
				{
					this->LoadModule(fpath, this);
				}
				else
				{
					this->AddInvalidModuleFile(fpath);
				}
			}
			iter++;
		}
	}

	void Host::AddInvalidModuleFile(std::string path)
	{
		// Don't add module twice
		std::vector<std::string>& invalid = this->invalid_module_files;
		if (std::find(invalid.begin(), invalid.end(), path) == invalid.end())
		{
			this->invalid_module_files.push_back(path);
		}
	}

	void Host::ScanInvalidModuleFiles()
	{
		ScopedLock lock(&moduleMutex);

		this->autoScan = false; // Do not recursively scan
		ModuleList modulesLoaded; // Track loaded modules

		std::vector<std::string>::iterator iter;
		iter = this->invalid_module_files.begin();
		while (iter != this->invalid_module_files.end())
		{
			std::string path = *iter;
			ModuleProvider *provider = FindModuleProvider(path);
			if (provider != NULL)
			{
				SharedPtr<Module> m = this->LoadModule(path, provider);

				// Module was loaded successfully
				if (!m.isNull())
					modulesLoaded.push_back(m);

				// Erase path, even on failure
				iter = invalid_module_files.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		if (modulesLoaded.size() > 0)
		{
			this->StartModules(modulesLoaded);

			/* If any of the invalid module files added
			 * a ModuleProvider, let them load their modules */
			this->ScanInvalidModuleFiles();
		}

		this->autoScan = true;
	}

	void Host::StartModules(ModuleList to_init)
	{
		ScopedLock lock(&moduleMutex);

		ModuleList::iterator iter = to_init.begin();
		while (iter != to_init.end())
		{
			(*iter)->Start();
			*iter++;
		}
	}

	SharedPtr<Module> Host::GetModule(std::string& name)
	{
		ScopedLock lock(&moduleMutex);
		ModuleMap::iterator iter = this->modules.find(name);
		if (this->modules.end() == iter) {
			return SharedPtr<Module>(NULL);
		}

		return iter->second;
	}

	bool Host::HasModule(std::string name)
	{
		ScopedLock lock(&moduleMutex);
		ModuleMap::iterator iter = this->modules.find(name);
		return (this->modules.end() != iter);
	}

	void Host::UnregisterModule(Module* module)
	{
		ScopedLock lock(&moduleMutex);

		std::cout << "Unregistering: " << module->GetName() << std::endl;

		// Remove from the module map
		ModuleMap::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			if (module == (i->second).get())
				break;
			i++;
		}

		// Remove from the list of loaded modules
		ModuleList::iterator j = this->loaded_modules.begin();
		while (j != this->loaded_modules.end())
		{
			if (module == (*j).get())
				break;
			j++;
		}

		module->Stop(); // Call Stop() lifecycle event

		if (i != this->modules.end())
			this->modules.erase(i);

		if (j != this->loaded_modules.end())
			this->loaded_modules.erase(j);
	}

	SharedPtr<StaticBoundObject> Host::GetGlobalObject() {
		return this->global_object;
	}

	bool Host::Start()
	{
		return true;
	}

	void Host::Stop ()
	{
	}

	int Host::Run()
	{
		if (args.size() > 1) {
			if (args.at(1) == "--attach-debugger") {
				printf("Waiting for debugger (Press Any Key to Continue)...\n");
				do {
					int c = getc(stdin);
					if (c > 0) break;
				} while (true);
			}
		}

		{
			ScopedLock lock(&moduleMutex);
			this->AddModuleProvider(this);
			this->LoadModules();
		}

		// allow start to immediately end
		this->running = this->Start();
		if (this->runUILoop) {
			while (this->running)
			{
				ScopedLock lock(&moduleMutex);
				if (!this->RunLoop())
				{
					break;
				}
			}
		}

		ScopedLock lock(&moduleMutex);
		this->Stop();
		this->UnloadModuleProviders();
		this->UnloadModules();

		// Clear the global object, being sure to remove the recursion, so
		// that the memory will be cleared
		this->global_object->Set(GLOBAL_NS_VARNAME, Value::Undefined);
		this->global_object = NULL;

		PRINTD("EXITING WITH EXITCODE = " << exitCode);
		return this->exitCode;
	}

	void Host::Exit(int exitcode)
	{
		ScopedLock lock(&moduleMutex);
		running = false;
		this->exitCode = exitcode;
	}
}
