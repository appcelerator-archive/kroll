/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_python_method.h"

namespace kroll
{
	KPythonMethod::KPythonMethod(PyObject *method) :
		method(method),
		object(new KPythonObject(method))
	{
		Py_INCREF(this->method);
	}

	KPythonMethod::~KPythonMethod()
	{
		Py_DECREF(this->method);
	}

	SharedValue KPythonMethod::Call(const ValueList& args)
	{
		PyObject *arglist = NULL;

		if (args.size() > 0)
		{
			arglist = PyTuple_New(args.size());
			for (size_t i = 0; i < args.size(); i++)
			{
				PyObject *pv = PythonUtils::ToPyObject(args[i]);
				PyTuple_SetItem(arglist, i, pv);
			}
		}

		PyObject *response = PyObject_CallObject(this->method, arglist);
		Py_XDECREF(arglist);

		SharedValue value = Value::Undefined;
		if (response == NULL && PyErr_Occurred() != NULL)
		{
			THROW_PYTHON_EXCEPTION
		}
		else if (response != NULL)
		{
			value = PythonUtils::ToKrollValue(response);
			Py_DECREF(response);
		}

		return value;
	}

	void KPythonMethod::Set(const char *name, SharedValue value)
	{
		this->object->Set(name, value);
	}

	SharedValue KPythonMethod::Get(const char *name)
	{
		return this->object->Get(name);
	}

	SharedStringList KPythonMethod::GetPropertyNames()
	{
		return this->object->GetPropertyNames();
	}

	PyObject* KPythonMethod::ToPython()
	{
		return this->object->ToPython();
	}

}
