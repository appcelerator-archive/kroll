/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _RUBY_MODULE_H
#define _RUBY_MODULE_H

#include <kroll/base.h>
#include <string>
#include <vector>
#include <ruby.h>

#undef sleep
#undef close
#ifdef RUBY_METHOD_FUNC
#  define VALUEFUNC(f) RUBY_METHOD_FUNC(f)
#  define VOIDFUNC(f) ((RUBY_DATA_FUNC) f)
#else
#  ifdef __cplusplus
#    if defined(_WIN32) && defined(_MSC_VER)
#      define VALUEFUNC(f) ((VALUE (*)())f)
#      define VOIDFUNC(f) ((void (*)(void *))f)
#    else
#      define VALUEFUNC(f) ((VALUE (*)(...))f)
#      define VOIDFUNC(f) ((void (*)(...))f)
#    endif
#  else
#   define VALUEFUNC(f) (f)
#   define VOIDFUNC(f) (f)
#  endif
#endif

#include <kroll/kroll.h>
#include "k_ruby_object.h"
#include "k_ruby_method.h"
#include "ruby_api.h"
#include "ruby_utils.h"
#include "ruby_evaluator.h"
#include "ruby_module_instance.h"
#include "ruby_unit_test_suite.h"

namespace kroll
{
	class RubyModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(RubyModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		void InitializeBinding();

		Host* GetHost()
		{
			return host;
		}
		static RubyModule* Instance()
		{
			return instance_;
		}
		virtual const char * GetDescription()
		{
			return "Ruby Module Loader";
		}


		// this is called by the ktest runner for unit testing the module
		void Test();

	private:
		SharedKObject binding;
		static RubyModule *instance_;
		DISALLOW_EVIL_CONSTRUCTORS(RubyModule);
	};
}

#endif
