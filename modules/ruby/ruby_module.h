/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _RUBY_MODULE_H
#define _RUBY_MODULE_H

#if defined(OS_OSX) || defined(OS_LINUX)
#define EXPORT __attribute__((visibility("default")))
#define KROLL_RUBY_API EXPORT
#elif defined(OS_WIN32)
# ifdef KROLL_RUBY_API_EXPORT
#  define KROLL_RUBY_API __declspec(dllexport)
# else
#  define KROLL_RUBY_API __declspec(dllimport)
# endif
#endif

#ifdef RUBY_METHOD_FUNC
#  define VALUEFUNC(f) RUBY_METHOD_FUNC(f)
#  define VOIDFUNC(f) ((RUBY_DATA_FUNC) f)
#else
#  if defined(OS_WIN32)
#    define VALUEFUNC(f) ((VALUE (*) (...))f)
#    define VOIDFUNC(f) ((void (*)(void *))f)
#  else
#    define VALUEFUNC(f) ((VALUE (*)(...))f)
#    define VOIDFUNC(f) ((void (*)(...))f)
#  endif
#endif

#include <kroll/base.h>
#include <string>
#include <vector>
#include <ruby.h>

#undef sleep
#undef close
#undef shutdown

#include <kroll/kroll.h>
#include "k_ruby_object.h"
#include "k_ruby_hash.h"
#include "k_ruby_method.h"
#include "k_ruby_list.h"
#include "ruby_utils.h"
#include "ruby_evaluator.h"
#include "ruby_module_instance.h"

namespace kroll
{
	class KROLL_RUBY_API RubyModule : public Module, public ModuleProvider
	{
	public:
		RubyModule(Host* host, const char* path) :
			Module(host, path, STRING(MODULE_NAME), STRING(MODULE_VERSION))
		{
		}

		~RubyModule()
		{
		}
		void Initialize();
		void Stop();

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

	private:
		KObjectRef binding;
		static RubyModule *instance_;
		DISALLOW_EVIL_CONSTRUCTORS(RubyModule);
	};
}

#endif
