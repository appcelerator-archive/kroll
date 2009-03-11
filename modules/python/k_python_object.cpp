/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_python_object.h"

namespace kroll
{
	KPythonObject::KPythonObject(PyObject *obj) :
		object(obj),
		read_only(false),
		delegate(NULL)
	{
		Py_INCREF(this->object);
		
	}

	KPythonObject::KPythonObject(PyObject *obj, bool read_only) :
		object(obj),
		read_only(read_only),
		delegate(new StaticBoundObject())
	{
		Py_INCREF(this->object);
		
	}

	KPythonObject::~KPythonObject()
	{
		Py_DECREF(this->object);
		this->object = NULL;
	}

	PyObject* KPythonObject::ToPython()
	{
		Py_INCREF(object);
		return this->object;
	}

	void KPythonObject::Set(const char *name, SharedValue value)
	{
		PyObject* py_value = PythonUtils::ToPyObject(value);

		if (read_only)
		{
			// This object is likely read-only, allow for binding
			// layer-only properties, even though this isn't a great idea.
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

	SharedValue KPythonObject::Get(const char *name)
	{
		if (0 == (PyObject_HasAttrString(this->object, (char*)name)))
		{
			if (this->read_only)
			{
				// Read-only objects can have binding layer properties
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

		SharedValue kroll_value = PythonUtils::ToKrollValue(value);
		Py_DECREF(value);
		return kroll_value;
	}

	SharedStringList KPythonObject::GetPropertyNames()
	{

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

