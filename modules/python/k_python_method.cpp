/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_python_method.h"

namespace kroll
{
	KPythonMethod::KPythonMethod(PyObject *obj, const char *n) :
		name(NULL), object(obj), delegate(new KPythonObject(obj))
	{
		if (n)
		{
			name = strdup(n);
		}
		Py_INCREF(this->object);
	}

	KPythonMethod::~KPythonMethod()
	{
		Py_DECREF(this->object);
		this->object = NULL;

		if (this->name)
		{
			free(this->name);
			this->name = NULL;
		}
	}

	SharedValue KPythonMethod::Call(const ValueList& args)
	{
		PyObject *arglist = NULL;
		if (args.size()>0)
		{
			arglist = PyTuple_New(args.size());
			for (int i = 0; i < (int) args.size(); i++)
			{
				SharedValue v = args[i];
				PyObject *pv = PythonUtils::ToPyObject(v);
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
		SharedValue value;
		if (response!=NULL)
		{
			value = PythonUtils::ToKrollValue(response,NULL);
		}
		else
		{
			value = Value::Undefined;
		}
		Py_XDECREF(response);
		return value;
	}

	void KPythonMethod::Set(const char *name, SharedValue value)
	{
		this->delegate->Set(name,value);
	}

	SharedValue KPythonMethod::Get(const char *name)
	{
		return this->delegate->Get(name);
	}

	SharedStringList KPythonMethod::GetPropertyNames()
	{
		return this->delegate->GetPropertyNames();
	}

}
