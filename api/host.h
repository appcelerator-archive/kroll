/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 *
 * a host container interface that end-os environments implement
 */
#ifndef _KR_HOST_H_
#define _KR_HOST_H_

namespace kroll
{
	class Module;

	/*
		Class: Host

	  Class that is implemented by the OS to handle OS-specific
	  loading and unloading of Kroll.
	 */
	class KROLL_API Host : public RefCounted, public ModuleProvider
	{
	public:
		typedef std::map<std::string,Module*> ModuleMap;

		/*
			Constructor: Host

			TODO: Document me
		*/
		Host(int argc, const char **argv);

	protected:
		virtual ~Host();

	public:
		/*
			Function: GetDescription

			TODO: Document me
		*/
		virtual const char * GetDescription() { return "Native module"; }

		/*
			Function: IsModule

			TODO: Document me
		*/
		virtual bool IsModule(std::string& path);

		/*
			Function: Run

			TODO: Document me
		*/
		virtual int Run() = 0;

		/*
			Function: FindModuleProvider

			TODO: Document me
		*/
		ModuleProvider* FindModuleProvider(std::string& filename);

		/*
			Function: FindModules

			TODO: Document me
		*/
		int FindModules (std::string &dir, std::vector<std::string> &files);

		/*
			Function: LoadModules

			TODO: Document me
		*/
		void LoadModules(std::vector<std::string>& paths);

		/*
			Function: RegisterModule

			TODO: Document me
		*/
		void RegisterModule(std::string& path, Module* module);

		/*
			Function: UnegisterModule

			TODO: Document me
		*/
		void UnregisterModule(Module* module);

		/*
			Function: GetModule

			TODO: Document me
		*/
		Module* GetModule(std::string& name);

		/*
			Function: HasModule

			TODO: Document me
		*/
		bool HasModule(std::string name);

		/*
			Function: GetModulesBegin

			TODO: Document me
		*/
		ModuleMap::iterator GetModulesBegin()
		{
			ScopedLock lock(&moduleMutex);
			return modules.begin();
		}

		/*
			Function: GetModulesEnd

			TODO: Document me
		*/
		ModuleMap::iterator GetModulesEnd()
		{
			ScopedLock lock(&moduleMutex);
			return modules.end();
		}

		/*
			Function: GetGlobalObject

			TODO: Document me
		*/
		SharedPtr<StaticBoundObject> GetGlobalObject();

		/*
			Function: GetApplicationHome

			TODO: Document me
		*/
		const std::string& GetApplicationHome() const { return appDirectory; }

		/*
			Function: GetRuntimeHome

			TODO: Document me
		*/
		const std::string& GetRuntimeHome() const { return runtimeDirectory; }

		/*
			Function: GetApplicationConfig

			TODO: Document me
		*/
		virtual const std::string& GetApplicationConfig() const { return appConfigPath; }

		/*
			Function: AddModuleProvider

			TODO: Document me
		*/
		void AddModuleProvider(ModuleProvider *provider) {
			ScopedLock lock(&moduleMutex);
			module_providers.push_back(provider);
			ScanInvalidModuleFiles();
		}

		/*
			Function: RemoveModuleProvider

			TODO: Document me
		*/
		void RemoveModuleProvider(ModuleProvider *provider) {
			ScopedLock lock(&moduleMutex);
			std::find(module_providers.begin(), module_providers.end(), provider);
			std::vector<ModuleProvider*>::iterator iter;
			iter = std::find(module_providers.begin(), module_providers.end(), provider);
			if (iter != module_providers.end()) {
				module_providers.erase(iter);
			}
		}

		/*
			Function: GetCommandLineArgCount

			TODO: Document me
		*/
		const int GetCommandLineArgCount() const {
			return argc;
		}

		/*
			Function: GetCommandLineArg

			TODO: Document me
		*/
		const char* GetCommandLineArg(int index) const {
			if (index >= argc) return NULL;
			return argv[index];
		}


	protected:
		int argc;
		const char **argv;
		ModuleMap modules;
		Mutex moduleMutex;
		std::vector<ModuleProvider *> module_providers;
		std::map<std::string,ModuleProvider*> module_creators;
		SharedPtr<StaticBoundObject> global_object;

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

	/**
	 * method that invokes a bound method on the main host thread
	 */
	extern KROLL_API Value* InvokeMethodOnMainThread(BoundMethod *method, ValueList* args);
}
#endif

