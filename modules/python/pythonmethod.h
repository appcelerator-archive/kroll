/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __PYTHON_METHOD_H__
#define __PYTHON_METHOD_H__

#include <Python.h>
#include "pythonvalue.h"
#include "pythonapi.h"

namespace kroll
{
	class KROLL_PYTHON_API PythonMethod : public BoundMethod
	{
	public:
		PythonMethod(PyObject *obj, const char *name);
		virtual ~PythonMethod();
		Value* Call(const ValueList& args);
		virtual void Set(const char *name, Value* value);
		virtual Value* Get(const char *name);
		virtual std::vector<std::string> GetPropertyNames();

		PyObject* ToPython() { Py_INCREF(object); return object; }

	private:
		char* name;
		PyObject* object;
	};
}

#endif

