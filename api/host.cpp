/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#endif
#if defined(OS_WIN32)
# include "base.h"
# include <windows.h>
#else
# include <dirent.h>
#endif
#include <errno.h>
#include <vector>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "kroll.h"
#include <Poco/DirectoryIterator.h>
#include <Poco/File.h>

namespace kroll
{
	Host::Host(int argc, const char *argv[])
	{
		char *ti_home = getenv("KR_HOME");
		char *ti_runtime = getenv("KR_RUNTIME");

#ifdef DEBUG		
		std::cout << ">>> KR_HOME=" << ti_home << std::endl;
		std::cout << ">>> KR_RUNTIME=" << ti_runtime << std::endl;
#endif

		if (ti_home == NULL)
		{
			std::cerr << "KR_HOME not defined, aborting." << std::endl;
			exit(1);
		}

		if (ti_runtime == NULL)
		{
			std::cerr << "KR_RUNTIME not defined, aborting." << std::endl;
			exit(1);
		}
		
		this->running = false;
		this->exitCode = 0;

		this->appDirectory = std::string(ti_home);
		this->runtimeDirectory = std::string(ti_runtime);
		this->global_object = new StaticBoundObject();

		// link the name of our global variable to ourself so
		//  we can reference from global scope directly to get it
		const char *name = GLOBAL_NS_VARNAME;
		SharedBoundObject b = global_object;
		SharedValue wrapper = Value::NewObject(b);
		this->global_object->Set(name, wrapper);

#if defined(OS_WIN32)
		this->module_suffix = "module.dll";
#elif defined(OS_OSX)
		this->module_suffix = "module.dylib";
#elif defined(OS_LINUX)
		this->module_suffix = "module.so";
#endif

		this->autoScan = false;

		// Sometimes libraries parsing argc and argv will
		// modify them, so we want to keep our own copy here
		for (int i = 0; i < argc; i++)
		{
			this->args.push_back(std::string(argv[i]));
		}
	}

	Host::~Host()
	{
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
		bool isModule = (filename.length() > module_suffix.length() && filename.substr(
				filename.length() - this->module_suffix.length()) == this->module_suffix);

		return isModule;
	}

	SharedPtr<Module> Host::LoadModule(std::string& path, ModuleProvider *provider)
	{
		KR_DUMP_LOCATION
		ScopedLock lock(&moduleMutex);

		SharedPtr<Module> module = NULL;
		try
		{
			module = provider->CreateModule(path);
			module->SetProvider(provider); // set the provider
			modules[path] = module; // add to the module map

			std::cout << "Module loaded " << module->GetName()
			          << " from " << path << std::endl;

			// Call module Load lifecycle event
			module->Initialize();
			
			// store module
			loaded_modules.push_back(module);
		}
		catch (kroll::ValueException& e)
		{
			SharedString s = e.GetValue()->DisplayString();
			std::cerr << "Error generated loading module ("
			          << path << "): " << *s << std::endl;
		}
		catch(...)
		{
			std::cerr << "Error generated loading module: " << path << std::endl;
		}

		return module;
	}

	void Host::UnloadModules()
	{
		KR_DUMP_LOCATION
		ScopedLock lock(&moduleMutex);
		
		while (this->loaded_modules.size() > 0)
		{
			SharedPtr<Module> module = this->loaded_modules.at(0);
			this->UnregisterModule(module);
		}
	}
	
	void Host::LoadModules()
	{
		KR_DUMP_LOCATION
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
		this->StartModules(this->modules);

		/* Try to load files that weren't modules
		 * using newly available module providers */
		this->ScanInvalidModuleFiles();

		/* From now on, adding a module provider will trigger
		 * a rescan of all invalid module files */
		this->autoScan = true;
	}

	void Host::FindBasicModules(std::string& dir)
	{
		KR_DUMP_LOCATION
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
		KR_DUMP_LOCATION
		ScopedLock lock(&moduleMutex);

		this->autoScan = false; // Do not recursively scan
		ModuleMap modulesLoaded; // Track loaded modules

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
					modulesLoaded[path] = m;

				// Erase path, even on failure
				invalid_module_files.erase(iter);
			}
			iter++;
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

	void Host::StartModules(ModuleMap to_init)
	{
		KR_DUMP_LOCATION
		ScopedLock lock(&moduleMutex);

		ModuleMap::iterator iter = to_init.begin();
		while (iter != to_init.end())
		{
			SharedPtr<Module> m = iter->second;
			m->Start();
			*iter++;
		}
	}

	SharedPtr<Module> Host::GetModule(std::string& name)
	{
		KR_DUMP_LOCATION
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
		KR_DUMP_LOCATION
		ScopedLock lock(&moduleMutex);
		ModuleMap::iterator iter = this->modules.begin();
		while (iter != this->modules.end())
		{
			SharedPtr<Module> other_module = iter->second;
			if (module == other_module.get())
			{
				std::cout << "Unregistering " << module->GetName()
				          << std::endl;
				module->Stop(); // Call Stop() lifecycle event
				this->modules.erase(iter); // SharedPtr will do the work
			}

			iter++;
		}
	}

	SharedPtr<StaticBoundObject> Host::GetGlobalObject() {
		return this->global_object;
	}
	
	bool Host::Start()
	{
		KR_DUMP_LOCATION
		return true;
	}
	
	void Host::Stop ()
	{
		KR_DUMP_LOCATION
	}

	int Host::Run()
	{
		KR_DUMP_LOCATION

		{
			ScopedLock lock(&moduleMutex);
			this->AddModuleProvider(this);
			this->LoadModules();

			// allow start to immediately end
			this->running = this->Start();
		}
		
		while (this->running)
		{
			ScopedLock lock(&moduleMutex);
			if (!this->RunLoop())
			{
				break; 
			}
		}
		
		ScopedLock lock(&moduleMutex);
		this->Stop();
		this->UnloadModules();
		this->UnloadModuleProviders();
		
		return this->exitCode;
	}
	
	void Host::Exit(int exitcode)
	{
		KR_DUMP_LOCATION
		ScopedLock lock(&moduleMutex);
		running = false;
		this->exitCode = exitcode;
	}
}
