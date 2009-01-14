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
	Value* PythonBoundObjectToValue(PyObject* value, const char *name = NULL);
	PyObject* ValueToPythonBoundObject(Value* value);
	PyObject* ValueListToPythonArray(const ValueList& list);
	const char * PythonStringToString(PyObject* value);
	PyObject* BoundMethodToPythonBoundObject(BoundMethod *method);
	PyObject* BoundObjectToPythonBoundObject(PyObject* self, PyObject* args, BoundObject *bo);
	void ThrowPythonException();
	void InitializeDefaultBindings(Host*);
}

#endif
