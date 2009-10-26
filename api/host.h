/*
 * Appcelerator Kroll - licensed under the Apache Public License 2 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_HOST_H_
#define _KR_HOST_H_
#include <Poco/Thread.h>
#include <Poco/FileStream.h>
#include <Poco/Timestamp.h>

namespace kroll
{
	class Module;
	typedef std::vector<SharedPtr<Module> > ModuleList;

	/**
	 * Class that is implemented by the OS to handle OS-specific
	 * loading and unloading of Kroll.
	 */
	class KROLL_API Host : public ModuleProvider
	{
		// FIXME: Is this still necessary?
		friend class Poco::ReleasePolicy<Host>;

	public:
		/**
		 * @param argc argument count
		 * @param argv command line arguments
		 */
		Host(int argc, const char** argv);

		/**
		 * Get the static host instance§§
		 */
		static SharedPtr<Host> GetInstance() { return instance_; }

		/**
		 * Called to run the host
		 */
		int Run();

		/**
		 * Called to exit the host and terminate the process
		 */
		virtual void Exit(int exitcode);

	 	/* Call with a method and arguments to invoke the method on the UI thread.
		 *
		 * @param method method to execute on the main thread
		 * @param args method arguments
		 * @param waitForCompletion block until method is finished (default: true)
		 *
		 * @return the method's return value§
		 */
		virtual KValueRef InvokeMethodOnMainThread(KMethodRef method,
			const ValueList& args, bool waitForCompletion=true) = 0;

		/**
		 * Add a module provider to the host
		 */
		void AddModuleProvider(ModuleProvider *provider);

		/**
		 * Remove a module provider
		 */
		void RemoveModuleProvider(ModuleProvider *provider);

		/**
		 * Call the Destroy() lifecycle event on this a module and
		 * remove it from our map of modules.
		 *
		 * @module The module to remove.
		 */
		void UnregisterModule(SharedPtr<Module> module);

		/**
		 * Get a module given the module path.
		 * @param path The path of the module to get
		 *
		 * @return A reference to the module.
		 */
		SharedPtr<Module> GetModuleByPath(std::string& path);
		
		/**
		 * Get a module give by the module name (such as tiui)
		 * @param name of the module
		 * @return A reference to the module
		 */
		SharedPtr<Module> GetModuleByName(std::string& name);

		/**
		 * @return whether or not a module with the path exists
		 * @param name the full path to the module
		*/
		bool HasModule(std::string name);

		/**
		 * @return the top-level API-holding object.
		 */
		KObjectRef GetGlobalObject() { return GlobalObject::GetInstance(); }

		/**
		 * @return the SharedApplication instance for this host
		 */
		SharedApplication GetApplication();

		/**
		 * @return The home directory for this application
		*/
		const std::string& GetApplicationHomePath();

		/**
		 * @return The directory of the current runtime§
		*/
		const std::string& GetRuntimePath();

		/**
		 * @return The id of the current application
		*/
		const std::string& GetApplicationID();

		/**
		 * @return The id of the current application
		*/
		const std::string& GetApplicationGUID();

		/**
		 * @return the number of command line arguments passed to this application
		 */
		const int GetCommandLineArgCount();

		/*
		 * @param index the argument index
		 * @return The command line argument at the given index
		*/
		const char* GetCommandLineArg(int index);

		/**
		 * @return true if the host is in debug mode, specified
		 * by the command line switch --debug
		 */
		bool IsDebugMode();

		/**
		 * @param path The filesystem path of a module
		 * @return true if the file is a native module (.dll / .dylib / .so)
		 */
		virtual bool IsModule(std::string& path);

		/**
		 * @return description for native modules
		*/
		virtual const char* GetDescription() { return "Native module"; }

		/**
		 * @return this host implementation's platform description.
		*/
		virtual const char* GetPlatform() { return "Base"; }

		/**
		 * @return this host implementation's module suffix.
		*/
		virtual const char* GetModuleSuffix() { return "unimplemented"; }

		/**
		 * @return whether or not this application runs the UI loop
		 */
		bool RunUILoop() { return runUILoop; }

		/**
		 * Set whether or not this application should run the UI loop
		 */
		void SetRunUILoop(bool runUILoop) { this->runUILoop = runUILoop; }
		
		/**
		 * return the elapsed time as microseconds since the host booted
		 */
		static Poco::Timestamp::TimeDiff GetElapsedTime()
		{
			return started_.elapsed();
		}

		/**
		 * Whether or not profiling is enabled for this Host
		 */
		bool ProfilingEnabled();

	protected:
		ModuleList loadedModules;

		Mutex moduleMutex;
		std::vector<ModuleProvider *> module_providers;
		std::vector<std::string> module_paths;
		KObjectRef globalObject;

		SharedApplication application;
		bool running;
		bool exiting;
		int exitCode;
		bool debug;
		bool waitForDebugger;
		bool autoScan;
		bool runUILoop;
		bool profile;
		std::string profilePath;
		std::string logFilePath;
		Poco::FileOutputStream *profileStream;
		bool consoleLogging;
		bool fileLogging;
		Logger* logger;

		virtual ~Host();

		/* This is the module suffix for this module provider. Since
		 * this is the basic provider the suffix is "module.(dll|dylib|so)"
		 * Other modules providers can override this property and use the
		 * default behavior of IsModule(). */
		std::string module_suffix;

		/* We store a cache of invalid module files so external providers
		   can re-query them without initiating a filesystem search */
		std::vector<std::string> invalid_module_files;

		/**
		 * Find the module provider for a given filename or return
		 * NULL if no module provider can be found.
		*/
		ModuleProvider* FindModuleProvider(std::string& filename);

		/**
		 * Load modules from all paths in invalid_module_files
		 * that can be loaded by module providers found in
		 * module_providers.
		*/
		void ScanInvalidModuleFiles();

		/**
		 * Copy a module's application-specific resources into the currently running app
		 */
		void CopyModuleAppResources(std::string& modulePath);

		/**
		 * Read / process a module's manifest file
		*/
		void ReadModuleManifest(std::string& modulePath);

		/**
		 * Load a modules from a path given a module provider.
		 *
		 * @param path Path to the module to attempt to load.
		 * @param provider The provider to attempt to load with.
		 *
		 * @return The module that was loaded or NULL on failure.
		*/
		SharedPtr<Module> LoadModule(std::string& path, ModuleProvider *provider);

		/**
		 * Do the initial round of module loading. First load all modules
		 * that can be loaded by the main Host module provider (shared
		 * libraries) and then load all modules which can be loaded by
		 * freshly installed module providers.
		*/
		void LoadModules();

		void UnloadModules();

		void UnloadModuleProviders();

		/**
		 * Scan a directory (no-recursion) for basic (shared library) modules
		 * and, if any are found, load them.
		 * @param dir The directory to scan.
		*/
		void FindBasicModules(std::string& dir);

		/**
		 * Call the Start() lifecycle event on a vector of modules.
		 * @param to_init A vector of modules to initialize.
		*/
		void StartModules(std::vector<SharedPtr<Module> > modules);

		virtual bool Start ();
		virtual bool RunLoop() = 0;
		virtual void Stop ();

		void AddInvalidModuleFile(std::string path);
		void SetupAppInstallerIfRequired();
		void ParseCommandLineArguments();
		static void AssertEnvironmentVariable(std::string);

	private:
		static SharedPtr<Host> instance_;
		static Poco::Timestamp started_;

		void SetupApplication(int argc, const char* argv[]);
		void SetupLogging();
		void SetupProfiling();
		void StopProfiling();

		DISALLOW_EVIL_CONSTRUCTORS(Host);
	};

	/**
	 * Call with a method and arguments to invoke the method on the UI thread.
	 * @param method method to execute on the main thread
	 * @param args method arguments
	 * @param waitForCompletion block until method is finished (default: true)
	 * @return the method's return value§
	 */
	extern KROLL_API KValueRef InvokeMethodOnMainThread(KMethodRef method, 
		const ValueList& args, bool waitForCompletion=true);
}
#endif

