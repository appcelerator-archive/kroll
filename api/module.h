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

	/**
	 * an interface that exposes a Kroll Module
	 */
	class KROLL_API Module : public RefCounted
	{
	public:
		Module(Host *host) : host(host) {} 
		virtual ~Module() {}

		virtual const char * GetName() = 0;
		virtual void Initialize() = 0;
		virtual void Destroy() = 0;
		void SetProvider(ModuleProvider* provider) { this->provider = provider; }
		ModuleProvider* GetProvider() { return provider; }
	
		virtual void Test() {}

	protected:
		Host *host;
		ModuleProvider *provider;
	private:
		DISALLOW_EVIL_CONSTRUCTORS(Module);
	};
}

//
// MACROS that are used to make it much easier to define
// and implement a module
//

using namespace kroll;
#define KROLL_MODULE_FACTORY_DEFINE(s) extern "C" EXPORT s* CreateModule(Host *host) \
{ \
	s *p = new s(host);\
	p->Initialize(); \
	return p; \
}  \
extern "C" EXPORT void DestroyModule(s* p)\
{\
	if (p) { \
		p->Destroy(); \
		RefCounted *r = (RefCounted*)p; \
		KR_DECREF(r); \
	} \
	p = 0; \
}\
const char* s::GetName() \
{ \
	return #s; \
}

typedef void* ModuleMethod;

#define KROLL_MODULE_CLASS(s) public: \
s(kroll::Host *host); \
virtual ~s(); \
const char* GetName(); \
void Initialize (); \
void Destroy ();

#define KROLL_MODULE_CONSTRUCTOR(s) s::s(Host *_host) : kroll::Module(_host)
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

