/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_HOST_H_
#define _KR_HOST_H_

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
	public:
		Host(int argc, const char** argv);

		/**
		 * Get the host singleton.
		 */
		static Host* GetInstance();

		/**
		 * Called to run the host
		 */
		int Run();

		/**
		 * Called to exit the host and terminate the process
		 */
		virtual void Exit(int exitcode);

		/*
		 * Call with a method and arguments to invoke the method on the UI thread.
		 * @param method method to execute on the main thread
		 * @param args method arguments
		 * @param waitForCompletion block until method is finished (default: true)
		 * @return the method's return value§
		 */
		KValueRef RunOnMainThread(KMethodRef method, const ValueList& args,
			bool waitForCompletion=true);

		/*
		 * Call with a method, thisObject, and  arguments to invoke the method on the UI thread.
		 * @param method method to execute on the main thread
		 * @param args method arguments
		 * @param waitForCompletion block until method is finished (default: true)
		 * @return the method's return value§
		 */
		virtual KValueRef RunOnMainThread(KMethodRef method, KObjectRef thisObject,
			const ValueList& args, bool waitForCompletion=true);

		/*
		 * Return true if this thread is the main thread.
		 */
		virtual bool IsMainThread() = 0;

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
		 * @param path The filesystem path of a module
		 * @return true if the file is a native module (.dll / .dylib / .so)
		 */
		virtual bool IsModule(std::string& path);

		/**
		 * Execute all jobs waiting to be run on the main thread.
		 */
		void RunMainThreadJobs();

		// TODO: We should eventually move away from sublcassing host.
		virtual const char* GetDescription() { return "Native module"; }
		virtual const char* GetPlatform() { return "Base"; }
		virtual const char* GetModuleSuffix() { return "unimplemented"; }

		inline SharedApplication GetApplication() { return this->application; }
		inline bool DebugModeEnabled() { return this->debug; }
		inline bool ProfilingEnabled() { return this->profile; }
		inline Poco::Timestamp::TimeDiff GetElapsedTime() { return timeStarted.elapsed(); }
		inline KObjectRef GetGlobalObject() { return GlobalObject::GetInstance(); }

	protected:
		ModuleList loadedModules;
		Mutex moduleMutex;
		std::vector<ModuleProvider *> moduleProviders;
		std::vector<std::string> modulePaths;
		SharedApplication application;
		bool exiting;
		int exitCode;
		bool debug;
		bool waitForDebugger;
		bool autoScan;
		bool profile;
		std::string profilePath;
		std::string logFilePath;
		Poco::FileOutputStream *profileStream;
		bool consoleLogging;
		bool fileLogging;
		Logger* logger;
		Poco::Timestamp timeStarted;

		void AssertEnvironmentVariable(std::string);
		ModuleProvider* FindModuleProvider(std::string& filename);
		void ScanInvalidModuleFiles();
		SharedPtr<Module> LoadModule(std::string& path, ModuleProvider *provider);
		void LoadModules();
		void UnloadModules();
		void UnloadModuleProviders();
		void FindBasicModules(std::string& dir);
		void StartModules(std::vector<SharedPtr<Module> > modules);

		virtual ~Host();
		virtual bool Start();
		virtual bool RunLoop() = 0;
		virtual void Stop();
		virtual void Shutdown();
		virtual void SignalNewMainThreadJob() { }

	private:
		Poco::Mutex jobQueueMutex;
		std::vector<MainThreadJob*> mainThreadJobs;
		std::vector<std::string> invalidModuleFiles;

		void SetupApplication(int argc, const char* argv[]);
		void SetupLogging();
		void SetupProfiling();
		void StopProfiling();
		void AddInvalidModuleFile(std::string path);
		void ParseCommandLineArguments();

		DISALLOW_EVIL_CONSTRUCTORS(Host);
	};

	KROLL_API KValueRef RunOnMainThread(KMethodRef method, const ValueList& args,
		bool waitForCompletion=true);
	KROLL_API KValueRef RunOnMainThread(KMethodRef method, KObjectRef thisObject,
		const ValueList& args, bool waitForCompletion=true);
	KROLL_API bool IsMainThread();
}
#endif

