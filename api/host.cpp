/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#endif
#if defined(OS_WIN32)
#include "base.h"
#include <windows.h>
#else
# include <dirent.h>
#endif
#include <cerrno>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>

#include "kroll.h"
#include "binding/profiled_bound_object.h"
#include <Poco/DirectoryIterator.h>
#include <Poco/File.h>
#include <Poco/Environment.h>
#include <Poco/AutoPtr.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Timespan.h>

#define HOME_ENV "KR_HOME"
#define APPID_ENV "KR_APP_ID"
#define APPGUID_ENV "KR_APP_GUID"
#define RUNTIME_ENV "KR_RUNTIME"
#define RUNTIME_HOME_ENV "KR_RUNTIME_HOME"
#define MODULES_ENV "KR_MODULES"
#define DEBUG_ENV "KR_DEBUG"

using Poco::Environment;

namespace kroll
{
	SharedPtr<Host> Host::instance_;
	Poco::Timestamp Host::started_;
	
	Host::Host(int argc, const char *argv[]) :
		running(false),
		exitCode(0),
		debug(false),
		waitForDebugger(false),
		autoScan(false),
		runUILoop(true),
		profile(false),
		profileStream(NULL)
	{
		instance_ = this;

		AssertEnvironmentVariable(HOME_ENV);
		AssertEnvironmentVariable(APPID_ENV);
		AssertEnvironmentVariable(APPGUID_ENV);
		AssertEnvironmentVariable(RUNTIME_ENV);
		AssertEnvironmentVariable(RUNTIME_HOME_ENV);
		AssertEnvironmentVariable(MODULES_ENV);

		if (Environment::has(DEBUG_ENV))
		{
			std::string debug_val = Environment::get(DEBUG_ENV);
			this->debug = (debug_val == "true" || debug_val == "yes" || debug_val == "1");
		}

#ifdef DEBUG
		this->debug = true; // if you compile in debug, force debug here
#endif

		this->appHomePath = Environment::get(HOME_ENV);
		this->appID = Environment::get(APPID_ENV);
		this->appGUID = Environment::get(APPGUID_ENV);
		this->runtimePath = Environment::get(RUNTIME_ENV);
		this->runtimeHomePath = Environment::get(RUNTIME_HOME_ENV);

		std::string paths = Environment::get(MODULES_ENV);
		//TI-180 make sure there is only one module specified
		FileUtils::Tokenize(paths, this->module_paths, KR_LIB_SEP, true);

		// check to see if we have to install the app
		SetupAppInstallerIfRequired();
		ParseCommandLineArguments(argc, argv);

		PRINTD(">>> " << HOME_ENV << "=" << this->appHomePath);
		PRINTD(">>> " << APPID_ENV << "=" << this->appID);
		PRINTD(">>> " << APPGUID_ENV << "=" << this->appGUID);
		PRINTD(">>> " << RUNTIME_ENV << "=" << this->runtimePath);
		PRINTD(">>> " << RUNTIME_HOME_ENV << "=" << this->runtimeHomePath);
		PRINTD(">>> " << MODULES_ENV << "=" << paths);
		
		// start profiling if turned on
		StartProfiling();

		// link the name of our global variable to ourself so
		// we can reference from global scope directly to get it
		if (this->profile)
		{
			// in the case of profiling, we wrap our top level global object
			// to use the profiled bound object which will profile all methods
			// going through this object and it's attached children
			this->global_object = new ProfiledBoundObject(GLOBAL_NS_VARNAME,new StaticBoundObject(),this->profileStream);
		}
		else
		{
			this->global_object = new StaticBoundObject();
		}
		this->global_object->SetObject(GLOBAL_NS_VARNAME,this->global_object);

		if (this->debug)
		{
			Logger::Initialize(true, true, Poco::Message::PRIO_DEBUG, this->appID);
		}
		else
		{
			Logger::Initialize(true, true, Poco::Message::PRIO_INFORMATION, this->appID);
		}
	}

	Host::~Host()
	{
	}

	void Host::StartProfiling()
	{
		if (this->profile)
		{
			Logger& logger = Logger::GetRootLogger();
			logger.Info("Starting Profiler. Logging going to %s",this->profilePath.c_str());
			this->profileStream = new Poco::FileOutputStream(this->profilePath);
		}
	}

	void Host::StopProfiling()
	{
		if (this->profile)
		{
			Logger& logger = Logger::GetRootLogger();
			logger.Info("Stopping Profiler");
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
			std::cerr << variable << " not defined, aborting." << std::endl;
			exit(1);
		}
	}

	void Host::ParseCommandLineArguments(int argc, const char** argv)
	{
		// Sometimes libraries parsing argc and argv will
		// modify them, so we want to keep our own copy here
		for (int i = 0; i < argc; i++)
		{
			std::string arg = argv[i];
			this->args.push_back(arg);
			PRINTD("ARGUMENT[" << i << "] => " << argv[i]);

			if (arg == "--debug")
			{
				std::cout << "DEBUGGING DETECTED!" << std::endl;
				this->debug = true;
			}
			else if (arg == "--attach-debugger")
			{
				this->waitForDebugger = true;
			}
			else if (arg.find("--profile=")==0)
			{
				std::string pp = arg.substr(10);
				if (pp.find("\"")==0)
				{
					pp = pp.substr(1,pp.length()-1);
				}
				this->profilePath = pp;
				this->profile = true;
			}
			else if (arg.find(STRING(_BOOT_HOME_FLAG)))
			{
				size_t i = arg.find("=");
				if (i != std::string::npos)
				{
					std::string newHome = arg.substr(i + 1);
					Environment::set(HOME_ENV, newHome);
					this->appHomePath = newHome;
				}
			}
		}
	}

	const std::string& Host::GetApplicationHomePath()
	{
		return this->appHomePath;
	}

	const std::string& Host::GetRuntimePath()
	{
		return this->runtimePath;
	}

	const std::string& Host::GetRuntimeHomePath()
	{
		return this->runtimeHomePath;
	}

	const std::string& Host::GetApplicationID()
	{
		return this->appID;
	}

	const std::string& Host::GetApplicationGUID()
	{
		return this->appGUID;
	}

	std::string Host::FindAppInstaller()
	{
		std::string appinstaller = FileUtils::Join(this->appHomePath.c_str(), "appinstaller", NULL);
		if (FileUtils::IsDirectory(appinstaller))
		{
			PRINTD("Found app installer at: " << appinstaller);
			return FileUtils::Trim(appinstaller);
		}

		appinstaller = FileUtils::Join(this->runtimePath.c_str(), "appinstaller", NULL);
		if (FileUtils::IsDirectory(appinstaller))
		{
			PRINTD("Found app installer at: " << appinstaller);
			return FileUtils::Trim(appinstaller);
		}
		
		PRINTD("Couldn't find app installer");
		return std::string();
	}

	void Host::SetupAppInstallerIfRequired()
	{
		std::string marker = FileUtils::Join(this->appHomePath.c_str(), ".installed", NULL);
		if (!FileUtils::IsFile(marker))
		{
			// not yet installed, look for the app installer
			std::string ai = FindAppInstaller();
			if (!ai.empty())
			{
				// make the app installer our new home so that the
				// app installer will run and set the environment var
				// to the special setting with the old home
				Environment::set("KR_APP_INSTALL_FROM",  this->appHomePath);
				this->appHomePath = ai;
				Environment::set(HOME_ENV, ai);
			}
			PRINTD("Application needs installation but no app installer found");
		}
	}

	bool Host::IsDebugMode()
	{
		return this->debug;
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
		PRINTD("CopyModuleAppResources: " << modulePath);
		std::string appDir = this->appHomePath;

		try
		{
			Poco::Path moduleDir(modulePath);
			moduleDir = moduleDir.parent();
			std::string mds(moduleDir.toString());

			const char* platform = this->GetPlatform();
			std::string resources_dir = FileUtils::Join(mds.c_str(), "AppResources", NULL);
			std::string plt_resources_dir = FileUtils::Join(resources_dir.c_str(), platform, NULL);
			std::string all_resources_dir = FileUtils::Join(resources_dir.c_str(), "all", NULL);
			Poco::File platformAppResourcesDir(plt_resources_dir);
			Poco::File allAppResourcesDir(all_resources_dir);

			if (platformAppResourcesDir.exists()
				&& platformAppResourcesDir.isDirectory())
			{

				std::vector<Poco::File> files;
				platformAppResourcesDir.list(files);
				for (size_t i = 0; i < files.size(); i++) 
				{
					Poco::File f = files.at(i);
					if (!f.exists())
					{
						PRINTD("Copying " << f.path() << " to " << appDir);
						f.copyTo(appDir);
					}
					else
					{
						PRINTD("SKIP Copying " << f.path() << " to " << appDir);
					}
				}
			}

			if (allAppResourcesDir.exists()
				&& allAppResourcesDir.isDirectory())
			{
				std::vector<Poco::File> files;
				allAppResourcesDir.list(files);
				for (size_t i = 0; i < files.size(); i++) 
				{
					Poco::File f = files.at(i);
					if (!f.exists())
					{
						PRINTD("Copying " << f.path() << " to " << appDir);
						f.copyTo(appDir);
					}
					else
					{
						PRINTD("SKIP Copying " << f.path() << " to " << appDir);
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
			PRINTD("Reading manifest for module: " << manifestPath.toString());

			Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> manifest = new Poco::Util::PropertyFileConfiguration(manifestFile.path());

			if (manifest->hasProperty("libpath")) 
			{
				PRINTD("libpath: " << modulePath);

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
					newLibPath += KR_LIB_SEP;
					newLibPath += FileUtils::Join(moduleTopDir.toString().c_str(), lib.c_str(), NULL);
				}

				PRINTD(libPathEnv << "=" << newLibPath);
				Environment::set(libPathEnv, newLibPath);
			}
		}
	}

	SharedPtr<Module> Host::LoadModule(std::string& path, ModuleProvider *provider)
	{
		ScopedLock lock(&moduleMutex);

		SharedPtr<Module> module = NULL;
		try
		{
			this->CopyModuleAppResources(path);
			this->ReadModuleManifest(path);
			module = provider->CreateModule(path);
			module->SetProvider(provider); // set the provider

			PRINTD("Loaded " << module->GetName()
			          << " (" << path << ")");

			// Call module Load lifecycle event
			module->Initialize();

			// Store module
			this->modules[path] = module;
			this->loaded_modules.push_back(module);
		}
		catch (kroll::ValueException& e)
		{
			SharedString s = e.GetValue()->DisplayString();
			std::cerr << "Error generated loading module ("
			          << path << "): " << *s << std::endl;
#ifdef OS_OSX
			KrollDumpStackTrace();
#endif
		}
		catch(std::exception &e)
		{
			std::cerr << "Error generated loading module (" << path <<"): " << e.what()<< std::endl;
#ifdef OS_OSX
			KrollDumpStackTrace();
#endif
		}
		catch(...)
		{
			std::cerr << "Error generated loading module: " << path << std::endl;
#ifdef OS_OSX
			KrollDumpStackTrace();
#endif
		}

		return module;
	}

	void Host::UnloadModules()
	{
		ScopedLock lock(&moduleMutex);

		// Stop all modules
		while (this->loaded_modules.size() > 0)
		{
			this->UnregisterModule(this->loaded_modules.at(0));
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

		/* All modules are now loaded, so start them all */
		this->StartModules(this->loaded_modules);

		/* Try to load files that weren't modules
		 * using newly available module providers */
		this->ScanInvalidModuleFiles();

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

	SharedPtr<Module> Host::GetModule(std::string& name)
	{
		ScopedLock lock(&moduleMutex);
		ModuleMap::iterator iter = this->modules.find(name);
		if (this->modules.end() == iter) 
		{
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

		PRINTD("Unregistering: " << module->GetName());

		// Remove from the module map
		ModuleMap::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			if (module == (i->second).get())
			{
				break;
			}
			i++;
		}

		// Remove from the list of loaded modules
		ModuleList::iterator j = this->loaded_modules.begin();
		while (j != this->loaded_modules.end())
		{
			if (module == (*j).get())
			{
				break;
			}
			j++;
		}

		module->Stop(); // Call Stop() lifecycle event

		if (i != this->modules.end())
		{
			this->modules.erase(i);
		}

		if (j != this->loaded_modules.end())
		{
			this->loaded_modules.erase(j);
		}
	}

	SharedPtr<KObject> Host::GetGlobalObject() 
	{
		return this->global_object;
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
			printf("Waiting for debugger (Press Any Key to Continue)...\n");
			getchar();
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
			PRINTD(*ss);
			return 1;
		}

		// allow start to immediately end
		this->running = this->Start();
		if (this->runUILoop) 
		{
			while (this->running)
			{
				ScopedLock lock(&moduleMutex);
				if (!this->RunLoop())
				{
					break;
				}
			}
		}

		ScopedLock lock(&moduleMutex);
		this->Stop();
		this->UnloadModuleProviders();
		this->UnloadModules();

		// Clear the global object, being sure to remove the recursion, so
		// that the memory will be cleared
		this->global_object->Set(GLOBAL_NS_VARNAME, Value::Undefined);
		this->global_object = NULL;

		// stop the profiling
		StopProfiling();

		PRINTD("EXITING WITH EXITCODE = " << exitCode);
		return this->exitCode;
	}

	void Host::Exit(int exitcode)
	{
		ScopedLock lock(&moduleMutex);
		running = false;
		this->exitCode = exitcode;

		// give our modules a hook for exit
		ModuleMap::iterator i = this->modules.begin();
		while (i != this->modules.end())
		{
			SharedPtr<Module> module = i->second;
			module->Exiting(exitcode);
			i++;
		}
	}
}
