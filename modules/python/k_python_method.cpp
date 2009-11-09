/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_python_method.h"

namespace kroll
{
	KPythonMethod::KPythonMethod(PyObject *method) :
		KMethod("Python.KMethod"),
		method(method),
		object(new KPythonObject(method))
	{
		PyLockGIL lock;
		Py_INCREF(this->method);
	}

	KPythonMethod::~KPythonMethod()
	{
		PyLockGIL lock;
		Py_DECREF(this->method);
	}

	KValueRef KPythonMethod::Call(const ValueList& args)
	{
		PyLockGIL lock;
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

		KValueRef value = Value::Undefined;
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

	void KPythonMethod::Set(const char *name, KValueRef value)
	{
		this->object->Set(name, value);
	}

	KValueRef KPythonMethod::Get(const char *name)
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

	bool KPythonMethod::Equals(KObjectRef other)
	{
		AutoPtr<KPythonMethod> pyOther = other.cast<KPythonMethod>();

		// This is not a Python object
		if (pyOther.isNull())
			return false;

		return pyOther->ToPython() == this->ToPython();
	}

}
