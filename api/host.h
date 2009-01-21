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
	typedef std::map<std::string,Module*> ModuleMap;

	/*
		Class: Host

	  Class that is implemented by the OS to handle OS-specific
	  loading and unloading of Kroll.
	 */
	class KROLL_API Host : public RefCounted, public ModuleProvider
	{
	public:
		/*
			Constructor: Host

			TODO: Document me
		*/
		Host(int argc, const char **argv);

	protected:
		virtual ~Host();

	public:

		/*
		 * Function: Run
		 *
		 * TODO: Document me
		 */
		virtual int Run() = 0;

		/*
		 * Function: AddModuleProvider
		 *
		 * TODO: Document me
		 */
		void AddModuleProvider(ModuleProvider *provider);

		/*
		 * Function: RemoveModuleProvider
		 *
		 * TODO: Document me
		 */
		void RemoveModuleProvider(ModuleProvider *provider);

		/*
		* Function: UnegisterModule
		*
			TODO: Document me
		*/
		void UnregisterModule(Module* module);

		/*
		 * Function: GetModule
		 *
		 * TODO: Document me
		 */
		Module* GetModule(std::string& name);

		/*
			Function: HasModule

			TODO: Document me
		*/
		bool HasModule(std::string name);

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
		 * Function: GetCommandLineArgCount
		 *
		 * TODO: Document me
		 */
		const int GetCommandLineArgCount();

		/*
		 * Function: GetCommandLineArg
		 *
		 * TODO: Document me
		*/
		const char* GetCommandLineArg(int index);

		/*
		 * Function: IsModule
		 *
		 * TODO: Document me
		 */
		virtual bool IsModule(std::string& path);

		/*
		 * Function: GetDescription
		 *
		 * TODO: Document me
		*/
		virtual const char * GetDescription() { return "Native module"; }


	protected:
		ModuleMap modules;
		Mutex moduleMutex;
		std::vector<ModuleProvider *> module_providers;
		std::vector<std::string> module_paths;
		SharedPtr<StaticBoundObject> global_object;
		std::vector<std::string> args;

		// This is the module suffix for this module provider. Since 
		// this is the basic provider the suffix is "module.(dll|dylib|so)"
		// Other modules providers can override this property and use the
		// default behavior of IsModule().
		std::string module_suffix;


		// we store a cache of invalid module files so external providers
		// can re-query them without initiating a filesystem search
		std::vector<std::string> invalid_module_files;

		/*
		 * Function: FindModuleProvider
		 *
		 * Find the module provider for a given filename or return
		 * NULL if no module provider can be found.
		*/
		ModuleProvider* FindModuleProvider(std::string& filename);

		/*
		 * Function: ScanInvalidModuleFiles
		 *
		 * Load modules from all paths in invalid_module_files
		 * that can be loaded by module providers found in
		 * module_providers.
		 *
		 * Parameters:
		 *  also_initialize - Whether to call the Initialize() lifecycle
		 *                    event when this method loads a module.
		*/
		void ScanInvalidModuleFiles(bool also_initialize=false);

		/*
		 * Function: LoadModule
		 *
		 * Load a modules from a path given a module provider.
		 *
		 * Parameters:
		 *  path - Path to the module to attempt to load.
		 *  provider - The provider to attempt to load with.
		 *
		 * Returns: The Module* that was loaded or NULL on failure.
		*/
		Module* LoadModule(std::string& path, ModuleProvider *provider);

		/*
		 * Function: LoadModules
		 *
		 * Do the initial round of module loading. First load all modules
		 * that can be loaded by the main Host module provider (shared 
		 * libraries) and then load all modules which can be loaded by
		 * freshly installed module providers.
		*/
		void LoadModules();

		/*
		 * Function: FindBasicModules
		 *
		 * Scan a directory (no-recursion) for basic (shared library) modules
		 * and, if any are found, load them.
		 *
		 * Parameters:
		 *  dir - The directory to scan.
		*/
		void FindBasicModules(std::string& dir);

		/*
		 * Function: InitializeModules
		 *
		 * Call the Initialize() lifecycle event on a vector of modules.
		 *
		 * Parameters:
		 *  to_init - A vector of modules to initialize.
		*/
		void InitializeModules(ModuleMap to_init);

	private:
		std::string appDirectory;
		std::string runtimeDirectory;
		std::string appConfigPath;
		bool basicModulesLoaded;

		DISALLOW_EVIL_CONSTRUCTORS(Host);
	};

	/**
	 * method that invokes a bound method on the main host thread
	 */
	extern KROLL_API Value* InvokeMethodOnMainThread(BoundMethod *method, ValueList* args);
}
#endif

