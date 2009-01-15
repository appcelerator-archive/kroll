/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_bound_method.h"

namespace kroll
{
	PythonBoundMethod::PythonBoundMethod(PyObject *obj, const char *n) : 
		name(NULL), object(obj), delegate(new PythonBoundObject(obj))
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
		KR_DECREF(this->delegate);
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
				PyObject *pv = PythonUtils::ToObject(v);
				PyTuple_SetItem(arglist, i, pv);
			}
		}
		PyObject *response = PyObject_CallObject(this->object,arglist);
		Py_XDECREF(arglist);

		if (PyErr_Occurred() != NULL)
		{
			Py_XDECREF(response);
			PythonUtils::ThrowException();
		}
		Value* value;
		if (response!=NULL)
		{
			value = PythonUtils::ToValue(response,NULL);
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
		this->delegate->Set(name,value);
	}

	Value* PythonBoundMethod::Get(const char *name)
	{
		return this->delegate->Get(name);
	}

	void PythonBoundMethod::GetPropertyNames(std::vector<const char *> *property_names)
	{
		this->delegate->GetPropertyNames(property_names);
	}

}
