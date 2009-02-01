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
		
		std::cout << ">>> KR_HOME=" << ti_home << std::endl;
		std::cout << ">>> KR_RUNTIME=" << ti_runtime << std::endl;

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

		this->appDirectory = std::string(ti_home);
		this->runtimeDirectory = std::string(ti_runtime);
		this->global_object = new StaticBoundObject();

		// link the name of our global variable to ourself so
		//  we can reference from global scope directly to get it
		const char *name = GLOBAL_NS_VARNAME;
		SharedBoundObject b = global_object;
		SharedValue wrapper = Value::NewObject(b);
		this->global_object->Set(name,wrapper);

#if defined(OS_WIN32)
		this->module_suffix = "module.dll";
#elif defined(OS_OSX)
		this->module_suffix = "module.dylib";
#elif defined(OS_LINUX)
		this->module_suffix = "module.so";
#endif

		this->basicModulesLoaded = false;

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

	void Host::AddModuleProvider(ModuleProvider *provider) {
		ScopedLock lock(&moduleMutex);
		module_providers.push_back(provider);
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

	void Host::RemoveModuleProvider(ModuleProvider *provider) {
		ScopedLock lock(&moduleMutex);

		std::vector<ModuleProvider*>::iterator iter =
		    std::find(module_providers.begin(),
		              module_providers.end(), provider);
		if (iter != module_providers.end()) {
			module_providers.erase(iter);
		}

	}

	bool Host::IsModule(std::string& filename)
	{
		bool isModule = (filename.length() > module_suffix.length() && filename.substr(
				filename.length() - this->module_suffix.length()) == this->module_suffix);

		return isModule;
	}

	SharedPtr<Module> Host::LoadModule(std::string& path, ModuleProvider *provider, bool *error)
	{
		ScopedLock lock(&moduleMutex);
		
		SharedPtr<Module> module;
		
		try
		{
			module = provider->CreateModule(path);
		}
		catch(...)
		{
			std::cerr << "Error generated loading module: " << path << std::endl;
		}

		if (!module.isNull())
		{
			module->SetProvider(provider); // set the provider
			modules[path] = module; // add to the module map

			std::cout << "Module loaded " << module->GetName()
			          << " from " << path << std::endl;

			// Call module Load lifecycle event
			module->Load();
		}
		else
		{
			std::cerr << "Could not load module from: " << path << std::endl;
			*error = true;
		}

		return module;
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

		/* From now on, adding a module provider triggers
 		 * a rescan of all invalid module files */
		this->basicModulesLoaded = true;

		/* Try to load files that weren't modules
		 * using newly available module providers */
		this->ScanInvalidModuleFiles();

		/* All modules are now loaded,
		 * so initialize them all */
		this->InitializeModules(this->modules);
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
					bool error = false;
					this->LoadModule(fpath, this, &error);
				}
				else if (std::find(this->invalid_module_files.begin(),this->invalid_module_files.end(),fpath)==this->invalid_module_files.end())
				{
					this->invalid_module_files.push_back(fpath);
				}
			}
			iter++;
		}
	}

	void Host::ScanInvalidModuleFiles(bool also_initialize)
	{
		ScopedLock lock(&moduleMutex);
		if (scanInProgress) return;
		
		scanInProgress = true;
		
		while (this->invalid_module_files.size() > 0)
		{
			// keep track in case we need to initialize
			ModuleMap modulesLoaded;
			std::map<std::string,bool> just_loaded;

			std::vector<std::string>::iterator iter;
			iter = this->invalid_module_files.begin();
			while (iter != this->invalid_module_files.end())
			{
				std::string path = *iter;
				// check to ensure that we're not in the same 
				// cycle of the module we just loaded so we 
				// don't reload again
				if (just_loaded[path] || path.length()==0)
				{
					iter++;
					continue;
				}
				ModuleProvider *provider = FindModuleProvider(path);
				if (provider != NULL)
				{
					bool error = false;
					SharedPtr<Module> m = this->LoadModule(path, provider, &error);
					if (!m.isNull())
					{
						just_loaded[path] = true;
						modulesLoaded[path] = m;
						invalid_module_files.erase(iter);
					}
					else if (error)
					{
						just_loaded[path] = true;
						invalid_module_files.erase(iter);
					}
				}
				iter++;
			}
		
			/* This happens when we scan invalid module files after
			 * the initial load of the application. This
			 * way all modules are initialized in waves. */
			if (also_initialize && modulesLoaded.size() > 0)
			{
				this->InitializeModules(modulesLoaded);
				continue;
			}
			
			break;
		}
		
		scanInProgress = false;
	}

	void Host::InitializeModules(ModuleMap to_init)
	{
		ScopedLock lock(&moduleMutex);

		ModuleMap::iterator iter = to_init.begin();
		while (iter != to_init.end())
		{
			SharedPtr<Module> m = iter->second;
			m->Initialize();
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
		ModuleMap::iterator iter = this->modules.begin();
		while (iter != this->modules.end())
		{
			SharedPtr<Module> other_module = iter->second;
			if (module == other_module.get())
			{
				std::cout << "Unregistering " << module->GetName()
				          << std::endl;
				module->Destroy(); // Call Destroy() lifecycle event
				this->modules.erase(iter); // SharedPtr will do the work
			}

			iter++;
		}
	}

	SharedPtr<StaticBoundObject> Host::GetGlobalObject() {
		return this->global_object;
	}
	
	void Host::Exit(int exitcode)
	{
		//FIXME: implement
	}
}
