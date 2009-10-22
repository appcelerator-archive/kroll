/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_python_object.h"

namespace kroll
{
	KPythonObject::KPythonObject(PyObject *obj) :
		KObject("Python.KPythonObject"),
		object(obj),
		readOnly(false),
		delegate(NULL)
	{
		PyLockGIL lock;
		Py_INCREF(this->object);
	}

	KPythonObject::KPythonObject(PyObject *obj, bool readOnly) :
		object(obj),
		readOnly(readOnly),
		delegate(new StaticBoundObject())
	{
		PyLockGIL lock;
		Py_INCREF(this->object);
	}

	KPythonObject::~KPythonObject()
	{
		PyLockGIL lock;
		Py_DECREF(this->object);
		this->object = NULL;
	}

	PyObject* KPythonObject::ToPython()
	{
		PyLockGIL lock;
		return this->object;
	}

	void KPythonObject::Set(const char *name, KValueRef value)
	{
		PyLockGIL lock;
		PyObject* py_value = PythonUtils::ToPyObject(value);

		if (readOnly)
		{
			// This object is likely read-only, allow for binding
			// layer-only properties, even though this isn't a great idea.
			PyAllowThreads allow;
			delegate->Set(name, value);
		}
		else
		{
			int result = PyObject_SetAttrString(this->object, (char*)name, py_value);
			if (result == -1 && PyErr_Occurred() != NULL)
			{
				THROW_PYTHON_EXCEPTION
			}
		}
	}

	KValueRef KPythonObject::Get(const char *name)
	{
		PyLockGIL lock;
		if (0 == (PyObject_HasAttrString(this->object, (char*)name)))
		{
			if (this->readOnly)
			{
				// Read-only objects can have binding layer properties
				PyAllowThreads allow;
				return delegate->Get(name);
			}
			else
			{
				return Value::Undefined;
			}
		}

		PyObject *value = PyObject_GetAttrString(this->object,(char*)name);
		if (value == NULL && PyErr_Occurred())
		{
			THROW_PYTHON_EXCEPTION
		}

		KValueRef kroll_value = PythonUtils::ToKrollValue(value);
		Py_DECREF(value);
		return kroll_value;
	}

	bool KPythonObject::Equals(KObjectRef other)
	{
		AutoPtr<KPythonObject> pyOther = other.cast<KPythonObject>();

		// This is not a Python object
		if (pyOther.isNull())
			return false;

		return pyOther->ToPython() == this->ToPython();
	}

	SharedStringList KPythonObject::GetPropertyNames()
	{
		PyLockGIL lock;
		SharedStringList property_names = new StringList();
		PyObject *props = PyObject_Dir(this->object);
		if (props == NULL)
			return property_names;

		PyObject *iterator = PyObject_GetIter(props);
		if (iterator == NULL)
			return property_names;

		PyObject *item;
		while ((item = PyIter_Next(iterator))) {
			property_names->push_back(new std::string(PythonUtils::ToString(item)));
			Py_DECREF(item);
		}

		Py_DECREF(iterator);
		Py_DECREF(props);
		return property_names;
	}
}

