/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_bound_method.h"

namespace kroll
{
	PythonBoundMethod::PythonBoundMethod(PyObject *obj, const char *n) : name(NULL), object(obj)
	{
		if (n)
		{
			name = strdup(n);
		}
		Py_INCREF(this->object);
	}

	PythonBoundMethod::~PythonBoundMethod()
	{
		Py_DECREF(this->object);
		this->object = NULL;
		if (this->name) free(this->name);
		this->name = NULL;
	}

	Value* PythonBoundMethod::Call(const ValueList& args)
	{
		PyObject *arglist = NULL;
		if (args.size()>0)
		{
			arglist = PyTuple_New(args.size());
			for (int i = 0; i < (int) args.size(); i++)
			{
				Value* v = args[i];
				PyObject *pv = ValueToPythonBoundObject(v);
				PyTuple_SetItem(arglist, i, pv);
			}
		}
		PyObject *response = PyObject_CallObject(this->object,arglist);
		Py_XDECREF(arglist);

		if (PyErr_Occurred() != NULL)
		{
			Py_XDECREF(response);
			ThrowPythonException();
		}
		Value* value;
		if (response!=NULL)
		{
			value = PythonBoundObjectToValue(response,NULL);
		}
		else
		{
			value = Value::Undefined();
		}
		Py_XDECREF(response);
		return value;
	}

	void PythonBoundMethod::Set(const char *name, Value* value)
	{
		int result = PyObject_SetAttrString(this->object,(char*)name,ValueToPythonBoundObject(value));
		KR_DECREF(value);

		PyObject *exception = PyErr_Occurred();
		if (result == -1 && exception != NULL)
		{
			ThrowPythonException();
		}
	}

	Value* PythonBoundMethod::Get(const char *name)
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

		Value* returnValue = PythonBoundObjectToValue(response,name);
		Py_DECREF(response);
		return returnValue;
	}

	void PythonBoundMethod::GetPropertyNames(std::vector<const char *> *property_names)
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
