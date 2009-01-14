/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _RUBY_MODULE_H
#define _RUBY_MODULE_H

#include <string>
#include <vector>
#include <iostream>
#include <ruby.h>
#include <kroll/kroll.h>

#include "ruby_api.h"
#include "ruby_types.h"
#include "ruby_bound_method.h"
#include "ruby_bound_object.h"
#include "ruby_bound_list.h"
#include "ruby_unit_test_suite.h"

namespace kroll
{
	class RubyModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(RubyModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		virtual const char * GetDescription() { return "Ruby Module Loader"; }

		Host* GetHost()
		{
			return host;
		}

		static RubyModule* Instance()
		{
			return instance_;
		}


		// this is called by the ktest runner for unit testing the module
		void Test();

	private:
		static RubyModule *instance_;
        DISALLOW_EVIL_CONSTRUCTORS(RubyModule);
	};
}

#include "ruby_module_instance.h"

#endif
