/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_bound_object.h"

namespace kroll
{
	PythonBoundObject::PythonBoundObject(PyObject *obj) : object(obj)
	{
		Py_INCREF(this->object);
	}

	PythonBoundObject::~PythonBoundObject()
	{
		Py_DECREF(this->object);
		this->object = NULL;
	}

	void PythonBoundObject::Set(const char *name, SharedPtr<Value> value)
	{
		int result = PyObject_SetAttrString(this->object,(char*)name,PythonUtils::ToObject(value));

		PyObject *exception = PyErr_Occurred();
		if (result == -1 && exception != NULL)
		{
			PythonUtils::ThrowException();
		}
	}

	SharedPtr<Value> PythonBoundObject::Get(const char *name)
	{
		// get should returned undefined if we don't have a property
		// named "name" to mimic what happens in Javascript
		if (0 == (PyObject_HasAttrString(this->object,(char*)name)))
		{
			return Value::Undefined;
		}

		PyObject *response = PyObject_GetAttrString(this->object,(char*)name);

		PyObject *exception = PyErr_Occurred();
		if (response == NULL && exception != NULL)
		{
			Py_XDECREF(response);
			PythonUtils::ThrowException();
		}

		SharedPtr<Value> returnValue = PythonUtils::ToValue(response,name);
		Py_DECREF(response);
		return returnValue;
	}

	SharedStringList PythonBoundObject::GetPropertyNames()
	{
		PyObject *props = PyObject_Dir(this->object);
		SharedStringList property_names(new StringList());

		if (props == NULL)
		{
			Py_DECREF(props);
			return property_names;
		}

		PyObject *iterator = PyObject_GetIter(props);
		PyObject *item;

		if (iterator == NULL)
			return property_names;

		while ((item = PyIter_Next(iterator))) {
			property_names->push_back(new std::string(PythonUtils::ToString(item)));
			Py_DECREF(item);
		}

		Py_DECREF(iterator);
		Py_DECREF(props);
		return property_names;
	}
}

