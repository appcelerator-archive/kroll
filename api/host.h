/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 *
 * a host container interface that end-os environments implement
 */
#ifndef _KR_HOST_H_
#define _KR_HOST_H_
#include <Poco/Thread.h>

namespace kroll
{
	class Module;
	typedef std::map<std::string, SharedPtr<Module> > ModuleMap;
	typedef std::vector<SharedPtr<Module> > ModuleList;

	/*
		Class: Host

	  Class that is implemented by the OS to handle OS-specific
	  loading and unloading of Kroll.
	 */
	class KROLL_API Host : public ModuleProvider
	{
		friend class Poco::ReleasePolicy<Host>;
	public:

		static const char * Platform;
		/*
			Constructor: Host

			TODO: Document me
		*/
		Host(int argc, const char **argv);

		static SharedPtr<Host> GetInstance() { return instance_; }

	protected:
		virtual ~Host();

	public:


		/*
		 * Function: Run
		 *
		 * called to run the host
		 */
		int Run();

		/**
		 * Function: Exit
		 *
		 * called to exit the host and terminate the process
		 */
		virtual void Exit(int exitcode);

		/*
		 * Function: InvokeMethodOnMainThread
		 *
		 * Call with a method and arguments to invoke the method
		 * on the main UI thread and return a value (blocking until run)
		 */
		virtual SharedValue InvokeMethodOnMainThread(
			SharedKMethod,
			const ValueList& args) = 0;

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
		 * Call the Destroy() lifecycle event on this a module and
		 * remove it from our map of modules.
		 *
		 * Parameters:
		 *    module - The module to remove.
		 */
		void UnregisterModule(Module* module);

		/*
		 * Function: GetModule
		 *
		 * Get a module given the module path.
		 *
		 * Parameters:
		 *    path - The path of the module to get
		 *
		 * Returns: A reference to the module.
		 */
		SharedPtr<Module> GetModule(std::string& path);

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

		/**
		 * Function: IsDebugMode
		 *
		 * returns true if the host is in debug mode, specified
		 * by the command line switch --debug
		 */
		bool IsDebugMode();

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
		virtual const char* GetDescription() { return "Native module"; }

		/*
		 * Function: GetPlatform
		 *
		 * Get this host implementation's platform description.
		*/
		virtual const char* GetPlatform() { return "Base"; }

		/*
		 * Function: GetModuleSuffix
		 *
		 * Get this host implementation's module suffix.
		*/
		virtual const char* GetModuleSuffix() { return "unimplemented"; }

		bool RunUILoop() { return runUILoop; }
		void SetRunUILoop(bool runUILoop) { this->runUILoop = runUILoop; }

	protected:
		ModuleMap modules;
		ModuleList loaded_modules;

		Mutex moduleMutex;
		std::vector<ModuleProvider *> module_providers;
		std::vector<std::string> module_paths;
		SharedPtr<StaticBoundObject> global_object;
		std::vector<std::string> args;
		bool runUILoop;

		/* This is the module suffix for this module provider. Since
		 * this is the basic provider the suffix is "module.(dll|dylib|so)"
		 * Other modules providers can override this property and use the
		 * default behavior of IsModule(). */
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
		*/
		void ScanInvalidModuleFiles();

		/*
		 * Function: CopyModuleAppResources
		 *
		 * Copy a module's application-specific resources into the currently running app
		 */
		void CopyModuleAppResources(std::string& modulePath);
		/*
			Function: ReadModuleManifest
			
			Read / process a module's manifest file
		*/
		void ReadModuleManifest(std::string& modulePath);

		/*
		 * Function: LoadModule
		 *
		 * Load a modules from a path given a module provider.
		 *
		 * Parameters:
		 *  path - Path to the module to attempt to load.
		 *  provider - The provider to attempt to load with.
		 *
		 * Returns: The module that was loaded or NULL on failure.
		*/
		SharedPtr<Module> LoadModule(std::string& path, ModuleProvider *provider);

		/*
		 * Function: LoadModules
		 *
		 * Do the initial round of module loading. First load all modules
		 * that can be loaded by the main Host module provider (shared
		 * libraries) and then load all modules which can be loaded by
		 * freshly installed module providers.
		*/
		void LoadModules();

		void UnloadModules();

		void UnloadModuleProviders();

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
		 * Function: StartModules
		 *
		 * Call the Start() lifecycle event on a vector of modules.
		 *
		 * Parameters:
		 *  to_init - A vector of modules to initialize.
		*/
		void StartModules(std::vector<SharedPtr<Module> > modules);

		virtual bool Start ();
		virtual bool RunLoop() = 0;
		virtual void Stop ();

		void AddInvalidModuleFile(std::string path);
		std::string SetupAppInstallerIfRequired(std::string home);
		std::string SetupStartPageOverrideIfRequired(int argc, const char**argv);
		std::string FindAppInstaller(std::string home);

	private:
		std::string appDirectory;
		std::string runtimeDirectory;
		std::string appConfigPath;
		bool autoScan;
		bool running;
		int exitCode;
		bool debug;

		static SharedPtr<Host> instance_;
		DISALLOW_EVIL_CONSTRUCTORS(Host);
	};

	/**
	 * method that invokes a bound method on the main host thread
	 */
	extern KROLL_API SharedValue InvokeMethodOnMainThread(SharedKMethod method, const ValueList& args);
}
#endif

