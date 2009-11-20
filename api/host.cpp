/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <cerrno>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "kroll.h"
#include "thread_manager.h"
#include <Poco/DirectoryIterator.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/Path.h>
#include <Poco/Environment.h>
#include <Poco/AutoPtr.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Timespan.h>

using Poco::File;
using Poco::Path;
using Poco::Environment;

#define HOME_ENV "KR_HOME"
#define RUNTIME_ENV "KR_RUNTIME"
#define MODULES_ENV "KR_MODULES"
#define DEBUG_ENV "KR_DEBUG"
#define DEBUG_ARG "--debug"
#define ATTACH_DEBUGGER_ARG "--attach-debugger"
#define NO_CONSOLE_LOG_ARG "--no-console-logging"
#define NO_FILE_LOG_ARG "--no-file-logging"
#define PROFILE_ARG "--profile"
#define LOGPATH_ARG "--logpath"
#define BOOT_HOME_ARG "--start"

namespace kroll
{
	static Host* hostInstance;

	Host::Host(int argc, const char *argv[]) :
		application(0),
		exiting(false),
		exitCode(0),
		debug(false),
		waitForDebugger(false),
		autoScan(false),
		profile(false),
		profileStream(0),
		consoleLogging(true),
		fileLogging(true),
		logger(0)
	{
		hostInstance = this;

#ifdef DEBUG
		this->debug = true;
#endif

		GlobalObject::Initialize();
		this->SetupApplication(argc, argv);
		this->ParseCommandLineArguments(); // Depends on this->application

		if (Environment::has(DEBUG_ENV))
		{
			std::string debug_val = Environment::get(DEBUG_ENV);
			this->debug = (debug_val == "true" || debug_val == "yes" || debug_val == "1");
		}

		this->SetupLogging(); // Depends on command-line arguments and this->debug
		this->SetupProfiling(); // Depends on logging
	}

	Host* Host::GetInstance()
	{
		return hostInstance;
	}

	void Host::SetupApplication(int argc, const char* argv[])
	{
		AssertEnvironmentVariable(HOME_ENV);
		AssertEnvironmentVariable(RUNTIME_ENV);
		AssertEnvironmentVariable(MODULES_ENV);

		string applicationHome(Environment::get(HOME_ENV));
		string runtimePath(Environment::get(RUNTIME_ENV));
		string modulePaths(Environment::get(MODULES_ENV));

		if (this->debug)
		{
			// Logging isn't activated yet -- need to just print here
			printf(">>> %s=%s\n", HOME_ENV, applicationHome.c_str());
			printf(">>> %s=%s\n", RUNTIME_ENV, runtimePath.c_str());
			printf(">>> %s=%s\n", MODULES_ENV, modulePaths.c_str());
		}

		this->application = Application::NewApplication(applicationHome);
		if (this->application.isNull())
		{
			std::cerr << "Could not load the application at: " << applicationHome << std::endl;
			exit(__LINE__);
		}
		this->application->SetArguments(argc, argv);

		// Re-resolve module/runtime dependencies of the application so that the
		// API module can introspect into the loaded components.
		this->application->ResolveDependencies();
		if (!this->application->runtime.isNull())
		{
			this->application->runtime->version = PRODUCT_VERSION;
		}

		// Parse the module paths, we'll later use this to load all the shared-objects.
		FileUtils::Tokenize(modulePaths, this->modulePaths, KR_LIB_SEP, true);
	}

	void Host::SetupLogging()
	{
		// Initialize the logger -- an empty logFilePath signfies no file logging,
		// but don't turn it off unless that was specified via the command-line.
		if (!this->fileLogging)
		{
			this->logFilePath = std::string();
		}
		else if (this->logFilePath.empty())
		{
			string dataDir = FileUtils::GetApplicationDataDirectory(this->application->id);
			this->logFilePath = FileUtils::Join(dataDir.c_str(), "tiapp.log", 0);
		}

		// If this application has no log level, we'll get a suitable default
		Logger::Level level = Logger::GetLevel(this->application->logLevel);
		Logger::Initialize(this->consoleLogging, this->logFilePath, level);
		this->logger = Logger::Get("Host");
	}

	Host::~Host()
	{
	}

	void Host::SetupProfiling()
	{
		if (this->profile)
		{
			// In the case of profiling, we wrap our top level global object
			// to use the profiled bound object which will profile all methods
			// going through this object and it's attached children
			this->profileStream = new Poco::FileOutputStream(this->profilePath);
			ProfiledBoundObject::SetStream(this->profileStream);
			GlobalObject::TurnOnProfiling();

			logger->Info("Starting Profiler. Output going to %s", this->profilePath.c_str());
		}
	}

	void Host::StopProfiling()
	{
		if (this->profile)
		{
			logger->Info("Stopping Profiler");
			ProfiledBoundObject::SetStream(0);
			profileStream->flush();
			profileStream->close();
			profileStream = 0;
			this->profile = false;
		}
	}

	void Host::AssertEnvironmentVariable(std::string variable)
	{
		if (!Environment::has(variable))
		{
			Logger* logger = Logger::Get("Host");
			logger->Fatal("required variable '%s' not defined, aborting.");
			exit(-999);
		}
	}

	void Host::ParseCommandLineArguments()
	{
		if (this->application->HasArgument(DEBUG_ARG))
		{
			this->debug = true;
		}

		if (this->application->HasArgument(ATTACH_DEBUGGER_ARG))
		{
			this->waitForDebugger = true;
		}

		if (this->application->HasArgument(NO_CONSOLE_LOG_ARG))
		{
			this->consoleLogging = false;
		}

		if (this->application->HasArgument(NO_FILE_LOG_ARG))
		{
			this->fileLogging = false;
		}

		if (this->application->HasArgument(PROFILE_ARG))
		{
			this->profilePath = this->application->GetArgumentValue(PROFILE_ARG);
			this->profile = !this->profilePath.empty();
		}

		if (this->application->HasArgument(LOGPATH_ARG))
		{
			this->logFilePath = this->application->GetArgumentValue(LOGPATH_ARG);
		}

		// Was this only used by the appinstaller? It complicates things a bit,
		// and the component list might not be correct after this point. -- Martin
		if (this->application->HasArgument(BOOT_HOME_ARG))
		{
			string newHome = this->application->GetArgumentValue(BOOT_HOME_ARG);
			SharedApplication newApp = Application::NewApplication(newHome);
			if (!newApp.isNull())
			{
				newApp->SetArguments(this->application->GetArguments());
				newApp->ResolveDependencies();
				if (!newApp->runtime.isNull())
				{
					newApp->runtime->version = PRODUCT_VERSION;
				}
				this->application = newApp;
			}
		}
	}

	void Host::AddModuleProvider(ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);
		moduleProviders.push_back(provider);

		if (autoScan)
		{
			this->ScanInvalidModuleFiles();
		}
	}

	/**
	 * Find the module provider for a given filename or return NULL if
	 * no module provider can be found.
	*/
	ModuleProvider* Host::FindModuleProvider(std::string& filename)
	{
		ScopedLock lock(&moduleMutex);

		std::vector<ModuleProvider*>::iterator iter;
		for (iter = moduleProviders.begin(); iter != moduleProviders.end(); iter++)
		{
			ModuleProvider *provider = (*iter);
			if (provider && provider->IsModule(filename))
			{
				return provider;
			}
		}
		return 0;
	}

	void Host::RemoveModuleProvider(ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);

		std::vector<ModuleProvider*>::iterator iter = std::find(
			moduleProviders.begin(), moduleProviders.end(), provider);
		if (iter != moduleProviders.end())
		{
			moduleProviders.erase(iter);
		}
	}

	void Host::UnloadModuleProviders()
	{
		while (moduleProviders.size() > 0)
		{
			ModuleProvider* provider = moduleProviders.at(0);
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

	/**
	 * Load a modules from a path given a module provider.
	 * @param path Path to the module to attempt to load.
	 * @param provider The provider to attempt to load with.
	 * @return The module that was loaded or NULL on failure.
	*/
	SharedPtr<Module> Host::LoadModule(std::string& path, ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);

		//TI-180: Don't load the same module twice
		SharedPtr<Module> module = this->GetModuleByPath(path);
		if (!module.isNull())
		{
			logger->Warn("Module cannot be loaded twice: %s", path.c_str());
			return 0;
		}

		try
		{
			logger->Debug("Loading module: %s", path.c_str());
			module = provider->CreateModule(path);
			module->SetProvider(provider); // set the provider
			module->Initialize();

			// loadedModules keeps track of the Module which is loaded from the
			// module shared-object, while application->modules holds the KComponent
			// metadata description of each module.
			this->loadedModules.push_back(module);
			this->application->UsingModule(module->GetName(), module->GetVersion(), path);

			logger->Info("Loaded module = %s", module->GetName().c_str());
		}
		catch (kroll::ValueException& e)
		{
			SharedString s = e.GetValue()->DisplayString();
			logger->Error("Could not load module (%s): %s", path.c_str(), s->c_str());
#ifdef OS_OSX
			KrollDumpStackTrace();
#endif
		}
		catch(std::exception &e)
		{
			string msg = e.what();
			logger->Error("Could not load module (%s): %s", path.c_str(), msg.c_str());
#ifdef OS_OSX
			KrollDumpStackTrace();
#endif
		}
		catch(...)
		{
			logger->Error("Could not load module (%s)", path.c_str());
#ifdef OS_OSX
			KrollDumpStackTrace();
#endif
		}

		return module;
	}

	void Host::UnloadModules()
	{
		ScopedLock lock(&moduleMutex);

		// Stop all modules before unloading them
		for (size_t i = 0; i < this->loadedModules.size(); i++)
		{
			this->loadedModules.at(i)->Stop();
		}

		// All modules are stopped now unloading them
		while (this->loadedModules.size() > 0)
		{
			this->UnregisterModule(this->loadedModules.at(0));
		}
	}

	/**
	 * Do the initial round of module loading. First load all modules  that can be
	 * loaded by the main Host module provider (shared libraries) and then load all
	 * modules which can be loaded by freshly installed module providers.
	*/
	void Host::LoadModules()
	{
		ScopedLock lock(&moduleMutex);

		/* Scan module paths for modules which can be
		 * loaded by the basic shared-object provider */
		std::vector<std::string>::iterator iter;
		iter = this->modulePaths.begin();
		while (iter != this->modulePaths.end())
		{
			this->FindBasicModules((*iter++));
		}

		/* Try to load files that weren't modules
		 * using newly available module providers */
		this->ScanInvalidModuleFiles();

		/* All modules are now loaded, so start them all */
		this->StartModules(this->loadedModules);

		/* From now on, adding a module provider will trigger
		 * a rescan of all invalid module files */
		this->autoScan = true;
	}

	/**
	 * Scan a directory (no-recursion) for shared-object modules and load them.
	*/
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
		std::vector<std::string>& invalid = this->invalidModuleFiles;
		if (std::find(invalid.begin(), invalid.end(), path) == invalid.end())
		{
			this->invalidModuleFiles.push_back(path);
		}
	}

	/**
	 * Load modules from all paths in invalidModuleFiles that can be loaded by
	 * module providers found in module_providers.
	*/
	void Host::ScanInvalidModuleFiles()
	{
		ScopedLock lock(&moduleMutex);

		this->autoScan = false; // Do not recursively scan
		ModuleList modulesLoaded; // Track loaded modules

		std::vector<std::string>::iterator iter;
		iter = this->invalidModuleFiles.begin();
		while (iter != this->invalidModuleFiles.end())
		{
			std::string path = *iter;
			ModuleProvider *provider = FindModuleProvider(path);
			if (provider != 0)
			{
				SharedPtr<Module> m = this->LoadModule(path, provider);

				// Module was loaded successfully
				if (!m.isNull())
					modulesLoaded.push_back(m);

				// Erase path, even on failure
				iter = invalidModuleFiles.erase(iter);
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

	/**
	 * Call the Start() lifecycle event on a vector of modules.
	*/
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

	SharedPtr<Module> Host::GetModuleByPath(std::string& path)
	{
		ScopedLock lock(&moduleMutex);
		ModuleList::iterator iter = this->loadedModules.begin();
		while (iter != this->loadedModules.end())
		{
			SharedPtr<Module> m = (*iter++);
			if (m->GetPath() == path)
				return m;
		}
		return 0;
	}

	SharedPtr<Module> Host::GetModuleByName(std::string& name)
	{
		ScopedLock lock(&moduleMutex);
		ModuleList::iterator iter = this->loadedModules.begin();
		while (iter != this->loadedModules.end())
		{
			SharedPtr<Module> m = (*iter++);
			if (m->GetName() == name)
				return m;
		}
		return 0;
	}

	void Host::UnregisterModule(SharedPtr<Module> module)
	{
		ScopedLock lock(&moduleMutex);

		logger->Notice("Unregistering: %s", module->GetName().c_str());
		module->Unload();

		ModuleList::iterator j = this->loadedModules.begin();
		while (j != this->loadedModules.end())
		{
			if (module == (*j).get())
			{
				j = this->loadedModules.erase(j);
			}
			else
			{
				j++;
			}
		}

		std::vector<SharedComponent>::iterator i = this->application->modules.begin();
		while (i != this->application->modules.end())
		{
			if (module->GetPath() == (*i)->path)
			{
				i = this->application->modules.erase(i);
			}
			else
			{
				i++;
			}
		}
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
		if (this->waitForDebugger)
		{
#ifdef OS_WIN32
			DebugBreak();
#else
			printf("Waiting for debugger (Press Any Key to Continue pid=%i)...\n", getpid());
			getchar();
#endif
		}

		try
		{
			ScopedLock lock(&moduleMutex);
			this->AddModuleProvider(this);
			this->LoadModules();
		}
		catch (ValueException e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			logger->Error(*ss);
			return 1;
		}

		// Do not run if exit flag is set. This can happen if a loaded
		// module from above called exit().
		if (this->exiting)
		{
			Host::Shutdown();
			return this->exitCode;
		}

		// Depending on the implementation of platform-specific host,
		// it may block in Start() or implement a UI loop which will
		// be continually called until it returns false.
		try
		{
			if (this->Start())
			{
				// We want the Host implementation to decide
				// when the best time to break out of the run
				// loop is. This allows it to handle additional
				// events if necessary.
				while (this->RunLoop()) {}
			}
		}
		catch (kroll::ValueException& e)
		{
			logger->Error("Caught exception in main loop: %s",
				e.ToString().c_str());
		}

		Host::Shutdown();
		return this->exitCode;
	}

	void Host::Shutdown()
	{
		// Do not shut down the logger here, because logging
		// may need to happen after this method is called.

		static bool shutdown = false;
		if (shutdown)
			return;

		ScopedLock lock(&moduleMutex);
		this->Stop();
		this->UnloadModuleProviders();
		this->UnloadModules();

		logger->Notice("Exiting with exit code: %i", exitCode);
		StopProfiling(); // Stop the profiler, if it was enabled
		Logger::Shutdown();
		shutdown = true;
	}

	void Host::Exit(int exitCode)
	{
		logger->Notice("Received exit signal (%d)", exitCode);
		if (GlobalObject::GetInstance()->FireEvent(Event::EXIT) &&
			GlobalObject::GetInstance()->FireEvent(Event::APP_EXIT))
		{
			this->exitCode = exitCode;
			this->exiting = true;
		}
		else
		{
			logger->Notice("Exit signal canceled by event handler");
		}
	}

	KValueRef Host::RunOnMainThread(KMethodRef method, const ValueList& args,
		bool waitForCompletion)
	{
		return this->RunOnMainThread(method, 0, args, waitForCompletion);
	}

	KValueRef Host::RunOnMainThread(KMethodRef method, KObjectRef thisObject,
		const ValueList& args, bool waitForCompletion)
	{
		MainThreadJob* job = new MainThreadJob(method, thisObject,
			args, waitForCompletion);
		if (this->IsMainThread() && waitForCompletion)
		{
			job->Execute();
		}
		else
		{
			Poco::ScopedLock<Poco::Mutex> s(jobQueueMutex);
			this->mainThreadJobs.push_back(job); // Enqueue job
		}

		this->SignalNewMainThreadJob();

		if (!waitForCompletion)
		{
			return Value::Undefined; // Handler will cleanup
		}
		else
		{
			// If this is the main thread, Wait() will fall
			// through because we've already called Execute() above.
			job->Wait();

			KValueRef result(job->GetResult());
			ValueException exception(job->GetException());
			delete job;

			if (!result.isNull())
				return result;
			else
				throw exception;
		}
	}

	void Host::RunMainThreadJobs()
	{
		// Prevent other threads trying to queue while we clear the queue.
		// But don't block the invocation task while we actually execute
		// the jobs -- one of these jobs may try to add something to the
		// job queue -- deadlock-o-rama
		std::vector<MainThreadJob*> jobs;
		{
			Poco::ScopedLock<Poco::Mutex> s(jobQueueMutex);
			jobs = this->mainThreadJobs;
			this->mainThreadJobs.clear();
		}

		for (size_t i = 0; i < jobs.size(); i++)
		{
			MainThreadJob* job = jobs[i];

			// Job might be freed soon after Execute(), so get this value now.
			bool asynchronous = !job->ShouldWaitForCompletion();
			job->Execute();

			if (asynchronous)
			{
				job->PrintException();
				delete job;
			}
		}
	}

	KValueRef RunOnMainThread(KMethodRef method, const ValueList& args,
		bool waitForCompletion)
	{
		return hostInstance->RunOnMainThread(method, args, waitForCompletion);
	}

	KValueRef RunOnMainThread(KMethodRef method, KObjectRef thisObject,
		const ValueList& args, bool waitForCompletion)
	{
		return hostInstance->RunOnMainThread(method, args, waitForCompletion);
	}

	bool IsMainThread()
	{
		return hostInstance->IsMainThread();
	}

}
