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
		//KR_DECREF(this->delegate);
	}

	SharedPtr<Value> PythonBoundMethod::Call(const ValueList& args)
	{
		PyObject *arglist = NULL;
		if (args.size()>0)
		{
			arglist = PyTuple_New(args.size());
			for (int i = 0; i < (int) args.size(); i++)
			{
				SharedPtr<Value> v = args[i];
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
		SharedPtr<Value> value;
		if (response!=NULL)
		{
			value = PythonUtils::ToValue(response,NULL);
		}
		else
		{
			value = Value::Undefined;
		}
		Py_XDECREF(response);
		return value;
	}

	void PythonBoundMethod::Set(const char *name, SharedPtr<Value> value)
	{
		this->delegate->Set(name,value);
	}

	SharedPtr<Value> PythonBoundMethod::Get(const char *name)
	{
		return this->delegate->Get(name);
	}

	SharedStringList PythonBoundMethod::GetPropertyNames()
	{
		return this->delegate->GetPropertyNames();
	}

}
