/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PLUGIN_H_
#define _KR_PLUGIN_H_

#include "module_provider.h"

namespace kroll
{
	class Host;

	/**
	 * An interface that represents a Kroll Module.
	 *
	 * The easiest way to create a module is to use the convenience macros defined in module.h.
	 * Example MyModule.h:
	 * \code
	 * #include <kroll/kroll.h>
	 * class MyModule : public kroll::Module
	 * {
	 *   KROLL_MODULE_CLASS(MyModule)
	 * }
	 * \endcode
	 *
	 * Example MyModule.cpp:
	 * \code
	 * #include "MyModule.h"
	 * KROLL_MODULE(MyModule);
	 *
	 * void MyModule::Initialize() {
	 *  // init code here..
	 * }
	 *
	 * void MyModule::Stop() {
	 *  // stop code here..
	 * }
	 * \endcode
	 */
	class KROLL_API Module
	{
	public:
		Module(Host *host, const char* inpath, const char* inname, const char* inversion) :
			host(host),
			path(std::string(inpath)),
			name(std::string(inname)),
			version(std::string(inversion))
		{
		}

		virtual ~Module()
		{
		}

		/**
		 * @return the path to this module's main directory
		 */
		std::string GetPath()
		{
			return this->path;
		}

		/*
		 * @return the name of the module
		 */
		std::string GetName()
		{
			return this->name;
		}

		/*
		 * @return the version of the module
		 */
		std::string GetVersion()
		{
			return this->version;
		}

		/**
		 * Called directly after module loading, during the loading
		 * process. Perform all initialization here that does not
		 * depend on the existence of other modules.
		 */
		virtual void Initialize() {};

		/**
		 * Called once all modules have been loaded. Use this to
		 * perform initialization that depends on the existence of
		 * other modules.
		 */
		virtual void Start() {};

		/**
		 * Called by the host when an exit has been requested to
		 * give modules a hook to perform certain pre-stop capabilities
		 *
		 * @param exitcode The code the process is exiting with
		 */
		virtual void Exiting(int exitcode) {}

		/**
		 * Called before the Host unregisters the module.
		 * Perform all unload cleanup here.
		 */
		virtual void Stop() {};

		/**
		 * Set the provider that created this module
		 * @param provider A module provider
		 */
		void SetProvider(ModuleProvider* provider)
		{
			this->provider = provider;
		}

		/**
		 * @return the provider that created this module
		 */
		const ModuleProvider* GetProvider()
		{
			return provider;
		}

	protected:
		Host *host;
		ModuleProvider *provider;

	private:
		std::string path;
		std::string name;
		std::string version;
		DISALLOW_EVIL_CONSTRUCTORS(Module);
	};
}

// MACROS that are used to make it much easier to define and implement a module
using namespace kroll;
typedef void* ModuleMethod;

/**
 * \def KROLL_MODULE(klass)
 * \brief A convenience macro for the module's implementation file.
 * \description Defines the default constructor, destructor, library implementations for the module "klass"
 */
#define KROLL_MODULE(ClassName, Name, Version) \
ClassName::ClassName(Host *host, const char* path, const char* name, const char* version) : \
	kroll::Module(host, path, name, version) \
{ \
} \
  \
ClassName::~ClassName() \
{ \
} \
  \
extern "C" EXPORT ClassName* CreateModule(Host *host, const char* path) \
{ \
	return new ClassName(host, path, Name, Version); \
}  \

/**
 * \def KROLL_MODULE_CLASS(klass)
 * \brief A convenience macro for the module's header file.
 * \description Defines the default constructor, destructor, and lifecycle methods for the module "klass"
 */
#define KROLL_MODULE_CLASS(ClassName) \
public: \
	ClassName(kroll::Host *host, const char* path, const char* name, const char* version); \
	virtual ~ClassName(); \
	void Initialize(); \
	void Stop();



#endif

