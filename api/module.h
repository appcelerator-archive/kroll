/**
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

	/*
	 * Class: Module
	 *
	 * an interface that exposes a Kroll Module
	 */
	class KROLL_API Module
	{
	public:
		/*
		 * Constructor: Module
		 */
		Module(Host *host, const char *path)
			 : host(host), path(std::string(path)) {} 
		virtual ~Module() {}

	public:

		/*
		 * Function: GetName
		 *
		 * Return the name of the module
		 */
		virtual const char* GetName() = 0;

		/*
		 *Function: Initialize
		 *
		 * Called directly after module loading, during the loading
		 * process. Perform all initialization here that does not
		 * depend on the existence of other modules.
		 */
		virtual void Initialize() {};

		/*
		 *Function: Start
		 *
		 * Called once all modules have been loaded. Use this to
		 * perform initialization that depends on the existence of
		 * other modules.
		 */
		virtual void Start() {};

		/*
		 * Function: Stop
		 *
		 * Called before the Host unregisters the module.
		 * Perform all unload cleanup here.
		 */
		virtual void Stop() {};

		/*
		 * Function: SetProvider
		 *
		 * Called to set the provider that created the module
		 */
		void SetProvider(ModuleProvider* provider)
		{
			this->provider = provider;
		}

		/*
		 * Function: GetProvider
		 * 
		 * Return the provider that created the module
		 */
		const ModuleProvider* GetProvider()
		{
			return provider;
		}

		
		/*
		 * Function: GetPath
		 *
		 * Return the path to the modules main directory
		 */
		const char *GetPath()
		{
			return path.c_str();
		}

		/*
		 * Function: Test
		 *
		 * Entry point for unit testing the module
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
#define KROLL_MODULE_FACTORY_DEFINE(s) extern "C" EXPORT s* CreateModule(Host *host, const char *path) \
{ \
	std::cout << "Creating module: " << #s << std::endl; \
	return new s(host,path);\
}  \
const char* s::GetName() \
{ \
	return #s; \
}

typedef void* ModuleMethod;

#define KROLL_MODULE_CLASS(s) public: \
s(kroll::Host *host, const char *path); \
virtual ~s(); \
const char* GetName(); \
void Initialize(); \
void Stop();

#define KROLL_MODULE_CONSTRUCTOR(s) s::s(Host *host, const char *path) : kroll::Module(host,path)
#define KROLL_MODULE_DESTRUCTOR(s) s::~s()


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

