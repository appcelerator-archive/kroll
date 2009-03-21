/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_PLUGIN_H_
#define _KR_PLUGIN_H_

#include "ref_counted.h"
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
		Module(Host *host, std::string path)
			 : host(host), path(path) {}
		virtual ~Module() {}

	public:

		/*
		 * @return the name of the module
		 */
		virtual const char* GetName() = 0;

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


		/**
		 * @return the path to this module's main directory
		 */
		const char *GetPath()
		{
			return path.c_str();
		}

		/**
		 * Entry point for unit testing the module.
		 * Module implementations should override.
		 */
		virtual void Test() {}

	protected:
		Host *host;
		ModuleProvider *provider;
	private:
		std::string path;
		DISALLOW_EVIL_CONSTRUCTORS(Module);
	};
}

//
// MACROS that are used to make it much easier to define
// and implement a module
//

using namespace kroll;
#define KROLL_MODULE_FACTORY_DEFINE(s) extern "C" EXPORT s* CreateModule(Host *host, const char* path) \
{ \
	return new s(host, std::string(path));\
}  \
const char* s::GetName() \
{ \
	return #s; \
}

typedef void* ModuleMethod;

/**
 * \def KROLL_MODULE_CLASS(klass)
 * \brief A convenience macro for the module's header file.
 * \description Defines the default constructor, destructor, and lifecycle methods for the module "klass"
 */
#define KROLL_MODULE_CLASS(s) public: \
s(kroll::Host *host, std::string path); \
virtual ~s(); \
const char* GetName(); \
void Initialize(); \
void Stop();

#define KROLL_MODULE_CONSTRUCTOR(s) s::s(Host *host, std::string path) : kroll::Module(host, path)
#define KROLL_MODULE_DESTRUCTOR(s) s::~s()

/**
 * \def KROLL_MODULE(klass)
 * \brief A convenience macro for the module's implementation file.
 * \description Defines the default constructor, destructor, library implementations for the module "klass"
 */
#define KROLL_MODULE(s) \
KROLL_MODULE_CONSTRUCTOR(s)\
{\
}\
\
KROLL_MODULE_DESTRUCTOR(s)\
{\
}\
\
KROLL_MODULE_FACTORY_DEFINE(s)\


#define KR_ASSERT(a) printf("[%s:%d] %s\n",__FILE__,__LINE__,(a?"PASS":"FAIL"))
#define KR_ASSERT_STR(a,b) KR_ASSERT(a && b && strcmp(a,b)==0)


#endif

