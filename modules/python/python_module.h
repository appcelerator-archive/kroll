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
#include "python_utils.h"
#include "k_python_object.h" 
#include "k_python_method.h"
#include "k_python_list.h"
#include "k_python_dict.h"
#include "python_evaluator.h"
#include "python_unit_test_suite.h"

#define THROW_PYTHON_EXCEPTION \
PyObject *_ptype, *_pvalue, *_trace; \
PyErr_Fetch(&_ptype, &_pvalue, &_trace); \
ValueException _ex = ValueException::FromString("Unknown"); \
if (_pvalue != NULL) \
{ \
	SharedValue _ex_val = PythonUtils::ToKrollValue(_pvalue); \
	_ex = ValueException(_ex_val); \
} \
Py_XDECREF(_ptype); \
Py_XDECREF(_pvalue); \
Py_XDECREF(_trace); \
throw _ex;

namespace kroll
{
	class PythonModule : public Module, public ModuleProvider
	{
		KROLL_MODULE_CLASS(PythonModule)

	public:
		virtual bool IsModule(std::string& path);
		virtual Module* CreateModule(std::string& path);
		void InitializeBinding();
		void Test();

		virtual const char * GetDescription() 
		{
			return "Python Module Loader";
		}
		Host* GetHost()
		{
			return host;
		}
		static PythonModule* Instance()
		{
			return instance_;
		}

	private:
		static PythonModule *instance_;
        DISALLOW_EVIL_CONSTRUCTORS(PythonModule);
	};
}

#include "python_module_instance.h"

#endif
