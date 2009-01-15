/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef PYTHON_TYPES_H_
#define PYTHON_TYPES_H_

#include <typeinfo>
#include "python_module.h"

namespace kroll
{
	class PythonUtils
	{
	public:
		static Value* ToValue(PyObject* value, const char *name = NULL);
		static PyObject* ToObject(Value* value);
		static PyObject* ToObject(const ValueList& list);
		static const char * ToString(PyObject* value);
		static PyObject* ToObject(BoundMethod *method);
		static PyObject* ToObject(PyObject* self, PyObject* args, BoundObject *bo);
		static void ThrowException();
		static void InitializeDefaultBindings(Host*);
	private:
		PythonUtils() {}
		~PythonUtils () {}
	};
}

#endif
