/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef PYTHON_TYPES_H_
#define PYTHON_TYPES_H_

#include <Python.h>
#include <api/binding/binding.h>

namespace kroll
{
	Value* PythonValueToValue(PyObject* value, const char *name);
	PyObject* ValueToPythonValue(Value* value);
	PyObject* ValueListToPythonArray(const ValueList& list);
	std::string PythonStringToString(PyObject* value);
	PyObject* BoundMethodToPythonValue(BoundMethod *method);
	PyObject* BoundObjectToPythonValue(PyObject* self, PyObject* args, BoundObject *bo);
	void ThrowPythonException();
	void InitializeDefaultBindings();
}

#endif
