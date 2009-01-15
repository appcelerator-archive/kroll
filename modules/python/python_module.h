/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PYTHON_MODULE_H
#define _PYTHON_MODULE_H

#include <string>
#include <vector>
#include <iostream>
#include <Python.h>
#include <kroll/kroll.h>

#include "python_api.h"
#include "python_types.h"
#include "python_bound_object.h" 
#include "python_bound_method.h"
#include "python_bound_list.h"
#include "python_unit_test_suite.h"

namespace kroll
{
	class PythonModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(PythonModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		virtual const char * GetDescription() { return "Python Module Loader"; }

		Host* GetHost()
		{
			return host;
		}

		static PythonModule* Instance()
		{
			return instance_;
		}


		// this is called by the ktest runner for unit testing the module
		void Test();

	private:
		static PythonModule *instance_;
        DISALLOW_EVIL_CONSTRUCTORS(PythonModule);
	};
}

#include "python_module_instance.h"

#endif
