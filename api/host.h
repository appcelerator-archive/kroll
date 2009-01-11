/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 *
 * a host container interface that end-os environments implement
 */
#ifndef _KR_HOST_H_
#define _KR_HOST_H_

#include "kroll.h"

namespace kroll
{
	class Module;

	/**
	 * Class that is implemented by the OS to handle OS-specific
	 * loading and unloading of Kroll.
	 */
	class KROLL_API Host : public RefCounted, public ModuleProvider
	{
	public:
		typedef std::map<std::string,Module*> ModuleMap;

		Host(int argc, const char **argv);
		virtual ~Host();
		virtual const char * GetDescription() { return "Native module"; }
		virtual bool IsModule(std::string& path);

		virtual int Run() = 0;

		ModuleProvider* FindModuleProvider(std::string& filename);
		int FindModules (std::string &dir, std::vector<std::string> &files);
		void LoadModules(std::vector<std::string>& paths);
		void RegisterModule(std::string& path, Module* module);
		void UnregisterModule(Module* module);
		Module* GetModule(std::string& name);
		bool HasModule(std::string name);
		ModuleMap::iterator GetModulesBegin()
		{
			ScopedLock lock(&moduleMutex);
			return modules.begin();
		}
		ModuleMap::iterator GetModulesEnd()
		{
			ScopedLock lock(&moduleMutex);
			return modules.end();
		}
		StaticBoundObject* GetGlobalObject();
		const std::string& GetApplicationHome() const { return appDirectory; }
		const std::string& GetRuntimeHome() const { return runtimeDirectory; }
		virtual const std::string& GetApplicationConfig() const { return appConfigPath; }

		void AddModuleProvider(ModuleProvider *provider) {
			ScopedLock lock(&moduleMutex);
			module_providers.push_back(provider);
			ScanInvalidModuleFiles();
		}

		void RemoveModuleProvider(ModuleProvider *provider) {
			ScopedLock lock(&moduleMutex);
			std::find(module_providers.begin(), module_providers.end(), provider);
			std::vector<ModuleProvider*>::iterator iter;
			iter = std::find(module_providers.begin(), module_providers.end(), provider);
			if (iter != module_providers.end()) {
				module_providers.erase(iter);
			}
		}

	protected:
		int argc;
		const char **argv;
		ModuleMap modules;
		Mutex moduleMutex;
		std::map<std::string,BoundObject*> bound_objects;
		std::vector<ModuleProvider *> module_providers;
		std::map<std::string,ModuleProvider*> module_creators;
		StaticBoundObject* global_object;

		// we store a cache of invalid module files so external providers
		// can re-query them without initiating a filesystem search
		std::vector<std::string> invalid_module_files;

		void ScanInvalidModuleFiles();

	private:
		std::string appDirectory;
		std::string runtimeDirectory;
		std::string appConfigPath;
		DISALLOW_EVIL_CONSTRUCTORS(Host);
	};
}
#endif

