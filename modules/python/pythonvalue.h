/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __PYTHON_VALUE_H__
#define __PYTHON_VALUE_H__

#include <Python.h>
#include <api/binding/binding.h>
#include "pythonapi.h"

namespace kroll
{
	class PythonValue : public BoundObject
	{
	public:
		PythonValue(PyObject *obj);
		virtual ~PythonValue();
		virtual void Set(const char *name, Value* value);
		virtual Value* Get(const char *name);
		virtual void GetPropertyNames(std::vector<std::string> *property_names);

		PyObject* ToPython() { Py_INCREF(object); return object; }

	protected:
		PyObject *object;
	};
}
#endif

