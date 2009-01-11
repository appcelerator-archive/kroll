/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _JAVASCRIPT_MODULE_H
#define _JAVASCRIPT_MODULE_H

#include <api/module.h>
#include <api/host.h>
#include <api/module_provider.h>
#include <string>

namespace kroll
{
	class JavascriptModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(JavascriptModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		virtual std::string GetDescription() { return "Javascript Module Loader"; }

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

	class JavascriptModuleInstance : public Module 
	{
	public:
		JavascriptModuleInstance(Host *host, std::string path_) :
			Module(host), path(path_)
		{
		}
		const char* GetName() { return path.c_str(); }
		void Initialize () {}
		void Destroy () {}
	protected:
		std::string path;
	};
}

#endif
