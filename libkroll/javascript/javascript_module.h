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

/* KJS <-> Kroll bindings */
namespace kroll 
{
	class KKJSObject;
	class KKJSMethod;
	class KKJSList;
	class JavaScriptModule;
	class JavaScriptModuleInstance;
}

#include "k_kjs_object.h"
#include "k_kjs_method.h"
#include "k_kjs_list.h"
#include "kjs_util.h"
#include "javascript_module_instance.h"
#include "javascript_methods.h"

namespace kroll
{
	class KROLL_API JavaScriptModule : public Module, public ModuleProvider
	{
	public:
		JavaScriptModule(Host* host, const char* path) :
			Module(host, path, STRING(MODULE_NAME), STRING(MODULE_VERSION))
		{

		}

		~JavaScriptModule()
		{

		}

		void Initialize();
		void Stop();
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);

		Host* GetHost()
		{
			return host;
		}

		static JavaScriptModule* Instance()
		{
			return instance;
		}


	private:
		static JavaScriptModule *instance;
	};
}
#endif
