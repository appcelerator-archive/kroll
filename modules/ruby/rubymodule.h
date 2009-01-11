/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _RUBY_MODULE_H
#define _RUBY_MODULE_H

#include <api/module.h>
#include <api/host.h>
#include <api/module_provider.h>
#include <string>
#include <ruby.h>

namespace kroll
{
	class RubyModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(RubyModule)

	protected:
		static RubyModule *instance_;
		int status;

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		virtual const char * GetDescription() { return "Ruby Module Loader"; }

		Host* GetHost() { return host; }
		static RubyModule* Instance() {
			return instance_;
		}
	};
}

#endif
