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
	SharedPtr<Host> Host::instance_;
	Poco::Timestamp Host::started_;

	Host::Host(int argc, const char *argv[]) :
		application(NULL),
		running(false),
		exiting(false),
		exitCode(0),
#ifdef DEBUG
		debug(true),
#else
		debug(false),
#endif
		waitForDebugger(false),
		autoScan(false),
		runUILoop(true),
		profile(false),
		profileStream(NULL),
		consoleLogging(true),
		fileLogging(true),
		logger(NULL)
	{
		instance_ = this;

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
		FileUtils::Tokenize(modulePaths, this->module_paths, KR_LIB_SEP, true);
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
			this->logFilePath = FileUtils::Join(dataDir.c_str(), "tiapp.log", NULL);
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
			profileStream = NULL;
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

	SharedApplication Host::GetApplication()
	{
		return this->application;
	}

	const std::string& Host::GetApplicationHomePath()
	{
		return this->application->path;
	}

	const std::string& Host::GetRuntimePath()
	{
		return this->application->runtime->path;
	}

	const std::string& Host::GetApplicationID()
	{
		return this->application->id;
	}

	const std::string& Host::GetApplicationGUID()
	{
		return this->application->guid;
	}

	bool Host::IsDebugMode()
	{
		return this->debug;
	}

	const int Host::GetCommandLineArgCount()
	{
		vector<string>& args = this->application->GetArguments();
		return args.size();
	}

	const char* Host::GetCommandLineArg(int index)
	{
		vector<string>& args = this->application->GetArguments();
		if ((int) args.size() > index)
		{
			return args.at(index).c_str();
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
			if (provider != NULL && provider->IsModule(filename))
			{
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
		if (iter != module_providers.end())
		{
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
		std::string suffix = this->GetModuleSuffix();
		bool isModule = (filename.length() > suffix.length() && filename.substr(
				filename.length() - suffix.length()) == suffix);

		return isModule;
	}

	void Host::CopyModuleAppResources(std::string& modulePath)
	{
		this->logger->Trace("CopyModuleAppResources: %s", modulePath.c_str());
		std::string appDir = this->application->path;
		Path appPath(this->application->path);

		try
		{
			Path moduleDir(modulePath);
			moduleDir = moduleDir.parent();
			std::string mds(moduleDir.toString());

			const char* platform = this->GetPlatform();
			std::string resources_dir = FileUtils::Join(mds.c_str(), "AppResources", NULL);
			std::string plt_resources_dir = FileUtils::Join(resources_dir.c_str(), platform, NULL);
			std::string all_resources_dir = FileUtils::Join(resources_dir.c_str(), "all", NULL);
			File platformAppResourcesDir(plt_resources_dir);
			File allAppResourcesDir(all_resources_dir);

			if (platformAppResourcesDir.exists()
				&& platformAppResourcesDir.isDirectory())
			{

				std::vector<File> files;
				platformAppResourcesDir.list(files);
				for (size_t i = 0; i < files.size(); i++)
				{
					File f = files.at(i);
					Path targetPath(appPath, Path(Path(f.path()).getBaseName()));
					File targetFile(targetPath);
					this->logger->Trace("target: %s", targetFile.path().c_str());
					if (!targetFile.exists())
					{
						this->logger->Trace("Copying : %s to %s", f.path().c_str(), appDir.c_str());
						f.copyTo(appDir);
					}
					else
					{
						this->logger->Trace("SKIP Copying : %s to %s", f.path().c_str(), appDir.c_str());
					}
				}
			}

			if (allAppResourcesDir.exists()
				&& allAppResourcesDir.isDirectory())
			{
				std::vector<File> files;
				allAppResourcesDir.list(files);
				for (size_t i = 0; i < files.size(); i++)
				{
					File f = files.at(i);
					Path targetPath(appPath, Path(Path(f.path()).getBaseName()));
					File targetFile(targetPath);
					this->logger->Trace("target: %s", targetFile.path().c_str());
					if (!targetFile.exists())
					{
						this->logger->Trace("Copying: %s to %s", f.path().c_str(), appDir.c_str());
						f.copyTo(appDir);
					}
					else
					{
						this->logger->Trace("SKIP Copying : %s to %s", f.path().c_str(), appDir.c_str());
					}
				}
			}
		}
		catch (Poco::Exception &exc)
		{
			// Handle..
		}
	}

	void Host::ReadModuleManifest(std::string& modulePath)
	{
		Poco::Path manifestPath(modulePath);
		Poco::Path moduleTopDir = manifestPath.parent();

		manifestPath = Poco::Path(FileUtils::Join(moduleTopDir.toString().c_str(), "manifest", NULL));

		Poco::File manifestFile(manifestPath);
		if (manifestFile.exists())
		{
			this->logger->Trace("Reading manifest for module: %s", manifestPath.toString().c_str());
			Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> manifest = new Poco::Util::PropertyFileConfiguration(manifestFile.path());

			if (manifest->hasProperty("libpath"))
			{
				std::string libPath = manifest->getString("libpath");
				Poco::StringTokenizer t(libPath, ",", Poco::StringTokenizer::TOK_TRIM);
	#if defined(OS_WIN32)
				std::string libPathEnv = "PATH";
	#elif defined(OS_OSX)
				std::string libPathEnv = "DYLD_LIBRARY_PATH";
	#elif defined(OS_LINUX)
				std::string libPathEnv = "LD_LIBRARY_PATH";
	#endif
				std::string newLibPath;

				if (Environment::has(libPathEnv))
				{
					newLibPath = Environment::get(libPathEnv);
				}

				for (size_t i = 0; i < t.count(); i++)
				{
					std::string lib = t[i];
					newLibPath = FileUtils::Join(moduleTopDir.toString().c_str(), lib.c_str(), NULL) +
						KR_LIB_SEP + newLibPath;
				}

				this->logger->Debug("%s=%s", libPathEnv.c_str(), newLibPath.c_str());
				Environment::set(libPathEnv, newLibPath);
			}
		}
	}

	SharedPtr<Module> Host::LoadModule(std::string& path, ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);

		//TI-180: Don't load the same module twice
		SharedPtr<Module> module = this->GetModuleByPath(path);
		if (!module.isNull())
		{
			logger->Warn("Module cannot be loaded twice: %s", path.c_str());
			return NULL;
		}

		try
		{
			logger->Debug("Loading module: %s", path.c_str());
			this->CopyModuleAppResources(path);
			this->ReadModuleManifest(path);
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

		/* Try to load files that weren't modules
		 * using newly available module providers */
		this->ScanInvalidModuleFiles();

		/* All modules are now loaded, so start them all */
		this->StartModules(this->loadedModules);

		/* From now on, adding a module provider will trigger
		 * a rescan of all invalid module files */
		this->autoScan = true;
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
		ScopedLock lock(&moduleMutex);

		this->autoScan = false; // Do not recursively scan
		ModuleList modulesLoaded; // Track loaded modules

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
					modulesLoaded.push_back(m);

				// Erase path, even on failure
				iter = invalid_module_files.erase(iter);
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
		return NULL;
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
		return NULL;
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

	bool Host::ProfilingEnabled()
	{
		return this->profile;
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

		// Depending on the implementation of platform-specific host,
		// it may block in Start() or implement a UI loop which will
		// be continually called until this->running becomes false.
		// Do not run if exit flag is set. This can happen if a loaded
		// module from above called exit().
		if (!this->exiting)
		{
			try
			{
				this->running = this->Start();
				if (this->runUILoop)
				{
					while (this->running)
					{
						if (!this->RunLoop())
						{
							break;
						}
					}
				}
			}
			catch (kroll::ValueException& e)
			{
				SharedString s = e.GetValue()->DisplayString();
				logger->Error("Caught exception in main loop: %s", s->c_str());
			}
		}

		ScopedLock lock(&moduleMutex);
		this->Stop();
		this->UnloadModuleProviders();
		this->UnloadModules();

		this->globalObject = NULL;

		// Stop the profiler, if it was enabled
		StopProfiling();

		logger->Notice("Exiting with exit code: %i", exitCode);

		Logger::Shutdown();
		return this->exitCode;
	}

	void Host::Exit(int exitCode)
	{
		logger->Notice("Received exit signal (%d)", exitCode);
		if (GlobalObject::GetInstance()->FireEvent(Event::EXIT) &&
			GlobalObject::GetInstance()->FireEvent(Event::APP_EXIT))
		{
			running = false;
			exiting = true;
			this->exitCode = exitCode;
		}
		else
		{
			logger->Notice("Exit signal canceled by event handler");
		}
	}
}
