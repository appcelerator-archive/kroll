/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _JAVASCRIPT_MODULE_H
#define _JAVASCRIPT_MODULE_H

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>
#include <cstring>
#include <map>

#include <kroll/kroll.h>
#include "javascript_api.h"
using namespace kroll;

/* KJS <-> Kroll bindings */
namespace kroll {
	class KJSKObject;
	class KJSKMethod;
	class KJSKList;
	class KJSUtil;
}
#include "kjs_kobject.h"
#include "kjs_kmethod.h"
#include "kjs_klist.h"
#include "kjs_util.h"

/* the KJS Javascript module provider */
namespace kroll {
	class JavascriptModule;
	class JavascriptModuleInstance;
	class JavascriptUnitTestSuite;
}
#include "javascript_module_instance.h"
#include "javascript_unit_test_suite.h"

namespace kroll
{
	class JavascriptModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(JavascriptModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		virtual const char * GetDescription() { return "Javascript Module Loader"; }

		Host* GetHost()
		{
			return host;
		}

		static JavascriptModule* Instance()
		{
			return instance;
		}

		// this is called by the ktest runner for unit testing the module
		void Test();

	private:
		static JavascriptModule *instance;
	};
}

#endif
