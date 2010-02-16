/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PYTHON_MODULE_H
#define _PYTHON_MODULE_H

#include <Python.h>
#include <kroll/kroll.h>
#include <string>
#include <vector>
#include <iostream>

#include "python_api.h"
#include "python_utils.h"
#include "k_python_object.h" 
#include "k_python_method.h"
#include "k_python_list.h"
#include "k_python_tuple.h"
#include "k_python_dict.h"
#include "python_evaluator.h"

#define THROW_PYTHON_EXCEPTION \
PyObject *_ptype, *_pvalue, *_trace; \
PyErr_Fetch(&_ptype, &_pvalue, &_trace); \
ValueException _ex = ValueException::FromString("Unknown"); \
if (_pvalue != NULL) \
{ \
	KValueRef _ex_val = PythonUtils::ToKrollValue(_pvalue); \
	_ex = ValueException(_ex_val); \
} \
Py_XDECREF(_ptype); \
Py_XDECREF(_pvalue); \
Py_XDECREF(_trace); \
throw _ex;

namespace kroll
{
	class KROLL_PYTHON_API PythonModule : public Module, public ModuleProvider
	{
	public:
		PythonModule(Host* host, const char* path) :
			Module(host, path, STRING(MODULE_NAME), STRING(MODULE_VERSION))
		{
		}

		~PythonModule()
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

		static PythonModule* Instance()
		{
			return instance_;
		}

	private:
		KObjectRef binding;
		static PythonModule *instance_;
		DISALLOW_EVIL_CONSTRUCTORS(PythonModule);
	};

	struct PyLockGIL
	{
		PyLockGIL() : gstate(PyGILState_Ensure())
		{ }

		~PyLockGIL()
		{
			PyGILState_Release(gstate);
		}

		PyGILState_STATE gstate;
	};

	struct PyAllowThreads
	{
		PyAllowThreads() : threadState(PyEval_SaveThread())
		{ }

		~PyAllowThreads()
		{
			PyEval_RestoreThread(threadState);
		}

		PyThreadState* threadState;
	};
}

#include "python_module_instance.h"

#endif
