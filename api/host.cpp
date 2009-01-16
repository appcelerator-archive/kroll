/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(OS_WIN32)
# include <windows.h>
#else
# include <dirent.h>
#endif

#include <Poco/DirectoryIterator.h>
#include <errno.h>
#include <vector>
#include <string>

#include "base.h"
#include "host.h"
#include "module.h"

namespace kroll
{
	Host::Host(int _argc, const char *_argv[]) :
		argc(_argc), argv(_argv)
	{
		char *ti_home = getenv("KR_HOME");
		char *ti_runtime = getenv("KR_RUNTIME");

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

		// link the name of our global variable to ourself so we can reference
		// from global scope directly to get it
		const char *name = GLOBAL_NS_VARNAME;
		Value *wrapper = new Value(this->global_object);
		this->global_object->Set(name,wrapper);
		KR_DECREF(wrapper);
	}

	Host::~Host()
	{
		KR_DECREF(this->global_object);
	}

	void Host::RegisterModule(std::string& path, Module* module) 
	{
		ScopedLock lock(&moduleMutex);
		KR_ADDREF(module);
		modules[path] = module;
	}

	void Host::UnregisterModule(Module* module) 
	{
		ScopedLock lock(&moduleMutex);
		std::map<std::string, Module*>::iterator iter = this->modules.find(
				module->GetName());
		if (this->modules.end() != iter) 
		{
			Module *p = iter->second;
			this->modules.erase(iter);
			KR_DECREF(p);
		}
		this->modules[module->GetName()] = module;
		KR_ADDREF(module);
	}

	Module* Host::GetModule(std::string& name) 
	{
		ScopedLock lock(&moduleMutex);
		std::map<std::string, Module*>::iterator iter = this->modules.find(name);
		if (this->modules.end() == iter) {
			return 0;
		}
		Module *module = iter->second;
		KR_ADDREF(module);
		return module;
	}

	bool Host::HasModule(std::string name) 
	{
		ScopedLock lock(&moduleMutex);
		std::map<std::string, Module*>::iterator iter = this->modules.find(name);
		return (this->modules.end() != iter);
	}

	void Host::LoadModule(const std::string path, ModuleProvider* provider)
	{
		ScopedLock lock(&moduleMutex);

		Module* module = p->CreateModule(path);
		if (module == NULL)
		{
			std::cerr << "Couldn't load module: "
			          << path << ", skipping..."
			          << std::endl;
			return;
		}

		printf("Module->SetProvider\n");
		module->SetProvider(provider);
		printf("Module->Load\n");
		module->Load();
		std::cout << "module loaded " << module->GetName()
		          << " from " << path << std::endl;
		

		this->RegisterModule(path, module);

		//we can now release our reference since the host has it
		KR_DECREF(module);
	}

	void Host::LoadModules(std::vector<std::string>& files) 
	{
		ScopedLock lock(&moduleMutex);
		std::cout << "have " << files.size() << " files" << std::endl;

		std::vector<std::string>::iterator iter = files.begin();
		for (; iter != files.end(); iter++) {
			std::string path = (*iter);
			std::cout << "loading next module: " << path << std::endl;

			// get the module factory
			Module* module = module_creators[path]->CreateModule(path);
			if (module==NULL)
			{
				std::cerr << "Couldn't load module: " << path << ", skipping..." << std::endl;
			}
			else
			{
				printf("Module->SetProvider\n");
				module->SetProvider(module_creators[path]);

				printf("Module->Load\n");
				module->Load();

				std::cout << "module loaded " << module->GetName() << " from " << path
						<< std::endl;
				
				// register our module
				this->RegisterModule(path, module);

				//we can now release our reference since the host has it
				KR_DECREF(module);
			}
		}
	}

	#if defined(OS_WIN32)
	std::string module_suffix = "module.dll";
	#elif defined(OS_OSX)
	std::string module_suffix = "module.dylib";
	#elif defined(OS_LINUX)
	std::string module_suffix = "module.so";
	#endif

	bool Host::IsModule(std::string& filename) 
	{
		bool isModule = (filename.length() > module_suffix.length() && filename.substr(
				filename.length() - module_suffix.length()) == module_suffix);

		std::cout << "IsModule? " << filename << " " << (isModule ? "true" : "false") << std::endl;
		return isModule;
	}

	ModuleProvider* Host::FindModuleProvider(std::string& filename) 
	{
		ScopedLock lock(&moduleMutex);
		if (IsModule(filename))
		{
			return this;
		}

		std::vector<ModuleProvider*>::iterator iter;
		for (iter = module_providers.begin(); iter != module_providers.end(); iter++) {
			ModuleProvider *provider = (*iter);
			if (provider != NULL && provider->IsModule(filename)) {
				//std::cout << "Found [" << provider->GetDescription()
				//		<< "] provider for module: " << filename << std::endl;
				return provider;
			}
		}
		return NULL;
	}

	int Host::FindModules() 
	{

		// Search the module directories for modules
		// that can be loaded by the basic module loader.
		std::vector<std::string>::iterator iter;
		for (iter = this->module_paths_begin;
		     iter != this->module_paths.end();
		     iter++)
		{
			this->FindBasicModules(*iter);
		}

		// Now scan invalid files and attempt to load
		// them with our new module loaders
		FindProvidedModules();

	}

	void Host::FindBasicModules(std::string &dir) 
	{
		ScopedLock lock(&moduleMutex);
		std::string search_dir = (dir);

		Poco::DirectoryIterator files(search_dir);
		Poco::DirectoryIterator end;
		while (files != end)
		{
			Poco::File file(files.name());

			// Skip hidden and non-file paths
			if (file.isDirectory() || file.isHidden())
				continue;

			const std::string path = file.path();

			if (IsModule(path))
				this->LoadModule(path, this);
		}
	}

	void FindProvidedModules()
	{
		std::vector<std::string>::iterator iter = invalid_module_files.begin();

		while (iter != invalid_module_files.end()) {
			ModuleProvider *provider = FindModuleProvider(*iter);
			if (provider != NULL) {
				const std::string path = *iter;
				this->LoadModule(path, p);
			}

			iter++;
		}
	}

	StaticBoundObject* Host::GetGlobalObject() {
		return this->global_object;
	}

}

// this is the platform specific code for main thread processing

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
@interface KrollMainThreadCaller : NSObject 
{
	kroll::BoundMethod *method;
	kroll::Value *result;
	kroll::ValueList* args;
}
- (id)initWithBoundMethod:(kroll::BoundMethod*)method args:(ValueList*)args;
- (void)call;
- (kroll::Value*)getResult;
@end

@implementation KrollMainThreadCaller
- (id)initWithBoundMethod:(kroll::BoundMethod*)m args:(ValueList*)a
{
	self = [super init];
	if (self)
	{
		method = m;
		args = a;
		result = nil;
		KR_ADDREF(method);
	}
	return self;
}
- (void)dealloc
{
	KR_DECREF(method);
	KR_DECREF(result);
	[super dealloc];
}
- (kroll::Value*)getResult
{
	return result;
}
- (void)call
{
	kroll::ValueList a;
	if (args)
	{
		ValueList::iterator i = args->begin();
		while (i!=args->end())
		{
			a.push_back((*i++));
		}
	}
	result = method->Call(a);
	KR_ADDREF(result);
}
@end
#endif

namespace kroll
{
	Value* InvokeMethodOnMainThread(BoundMethod *method, ValueList* args)
	{
#ifdef OS_OSX
	    KrollMainThreadCaller *caller = [[KrollMainThreadCaller alloc] initWithBoundMethod:method args:args];
	    [caller performSelectorOnMainThread:@selector(call) withObject:nil waitUntilDone:YES];
		Value *result = [caller getResult];
		// make sure to return a new reference because we'll release it
		// when we release the caller
		if (result) KR_ADDREF(result);
		[caller release];
#else
		//FIXME - implement for Win32 and Linux. Until then...we 
		//will just forward on same thread 
		std::cerr << "WARNING: Invoking method on non-main Thread!" << std::endl;
		Value *result = method->Call(*args);
#endif
		return result;
	}
}
