/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "pythontypes.h"
#include "pythonmethod.h"

namespace kroll
{
	PythonMethod::PythonMethod(PyObject *obj, const char *n) : name(NULL), object(obj)
	{
		if (n)
		{
			name = strdup(n);
		}
		Py_INCREF(this->object);
	}

	PythonMethod::~PythonMethod()
	{
		Py_DECREF(this->object);
		this->object = NULL;
		if (this->name) free(this->name);
		this->name = NULL;
	}

	Value* PythonMethod::Call(const ValueList& args)
	{
		PyObject *arglist = NULL;
		if (args.size()>0)
		{
			arglist = PyTuple_New(args.size());
			for (int i = 0; i < (int) args.size(); i++)
			{
				Value* v = args[i];
				PyObject *pv = ValueToPythonValue(v);
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
			value = PythonValueToValue(response,NULL);
		}
		else
		{
			value = Value::Undefined();
		}
		Py_XDECREF(response);
		return value;
	}

	void PythonMethod::Set(const char *name, Value* value)
	{
		int result = PyObject_SetAttrString(this->object,(char*)name,ValueToPythonValue(value));
		KR_DECREF(value);

		PyObject *exception = PyErr_Occurred();
		if (result == -1 && exception != NULL)
		{
			ThrowPythonException();
		}
	}

	Value* PythonMethod::Get(const char *name)
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

	std::vector<std::string> PythonMethod::GetPropertyNames()
	{
		std::vector<std::string> names;
		PyObject *props = PyObject_Dir(this->object);

		if (props == NULL)
		{
			Py_DECREF(props);
			return names;
		}

		PyObject *iterator = PyObject_GetIter(props);
		PyObject *item;

		if (iterator == NULL)
			return names;

		while ((item = PyIter_Next(iterator))) {
			names.push_back(PythonStringToString(item));
			Py_DECREF(item);
		}

		Py_DECREF(iterator);
		Py_DECREF(props);
		return names;
	}
	
}
