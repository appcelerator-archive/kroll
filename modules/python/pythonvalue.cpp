/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "pythonvalue.h"
#include "pythontypes.h"
#include <vector>

namespace kroll
{
	PythonValue::PythonValue(PyObject *obj) : object(obj)
	{
		Py_INCREF(this->object);
	}

	PythonValue::~PythonValue()
	{
		Py_DECREF(this->object);
		this->object = NULL;
	}

	void PythonValue::Set(const char *name, Value* value)
	{
		int result = PyObject_SetAttrString(this->object,(char*)name,ValueToPythonValue(value));

		PyObject *exception = PyErr_Occurred();
		if (result == -1 && exception != NULL)
		{
			ThrowPythonException();
		}
	}

	Value* PythonValue::Get(const char *name)
	{
		// get should returned undefined if we don't have a property
		// named "name" to mimic what happens in Javascript
		if (0 == (PyObject_HasAttrString(this->object,(char*)name)))
		{
			return Value::Undefined();
		}

		PyObject *response = PyObject_GetAttrString(this->object,(char*)name);

		PyObject *exception = PyErr_Occurred();
		if (response == NULL && exception != NULL)
		{
			Py_XDECREF(response);
			ThrowPythonException();
		}

		Value* returnValue = PythonValueToValue(response,name);
		Py_DECREF(response);
		return returnValue;
	}

	void PythonValue::GetPropertyNames(std::vector<const char *> *property_names)
	{
		PyObject *props = PyObject_Dir(this->object);

		if (props == NULL)
		{
			Py_DECREF(props);
			return;
		}

		PyObject *iterator = PyObject_GetIter(props);
		PyObject *item;

		if (iterator == NULL)
			return;

		while ((item = PyIter_Next(iterator))) {
			property_names->push_back(PythonStringToString(item));
			Py_DECREF(item);
		}

		Py_DECREF(iterator);
		Py_DECREF(props);
	}
}

