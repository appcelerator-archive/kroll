/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_python_list.h"

namespace kroll
{
	KPythonList::KPythonList(PyObject *list) :
		list(list),
		object(new KPythonObject(list, true))
	{
		PythonGILState gil();
		if (!PyList_Check(list))
			throw ValueException::FromString("Invalid PyObject passed. Should be a Py_List");
		else
			Py_INCREF(this->list);
	}

	KPythonList::~KPythonList()
	{
		PythonGILState gil();
		Py_DECREF(this->list);
	}

	void KPythonList::Append(SharedValue value)
	{
		PythonGILState gil();
		PyObject* py_value = PythonUtils::ToPyObject(value);
		PyList_Append(this->list, py_value);
	}

	unsigned int KPythonList::Size()
	{
		PythonGILState gil();
		return PyList_Size(this->list);
	}

	bool KPythonList::Remove(unsigned int index)
	{
		PythonGILState gil();
		if (index < this->Size())
		{
			PyObject* empty_list = PyList_New(0);
			PyList_SetSlice(this->list, index, index + 1, empty_list);
			Py_DECREF(empty_list);
			return true;
		}
		else
		{
			return false;
		}
	}

	SharedValue KPythonList::At(unsigned int index)
	{
		PythonGILState gil();
		if (index >= 0 && index < this->Size())
		{
			PyObject *p = PyList_GetItem(this->list, index);
			SharedValue v = PythonUtils::ToKrollValue(p);
			Py_DECREF(p);
			return v;
		}
		else
		{
			return Value::Undefined;
		}
	}

	void KPythonList::Set(const char *name, SharedValue value)
	{
		// Check for integer value as name
		int index = -1;
		if (KList::IsInt(name) && ((index = atoi(name)) >= 0))
		{
			this->SetAt((unsigned int) index, value);
		}
		else
		{
			this->object->Set(name, value);
		}
	}

	void KPythonList::SetAt(unsigned int index, SharedValue value)
	{
		PythonGILState gil();
		while (index >= this->Size())
		{
			// now we need to create entries between current size
			// and new size and make the entries undefined.
			this->Append(Value::Undefined);
		}
		PyObject* py_value = PythonUtils::ToPyObject(value);
		PyList_SetItem(this->list, index, py_value);
		return;
	}

	SharedValue KPythonList::Get(const char *name)
	{
		if (KList::IsInt(name))
		{
			unsigned int index = (unsigned int) atoi(name);
			if (index >= 0)
				return this->At(index);
		}
		return object->Get(name);
	}

	SharedStringList KPythonList::GetPropertyNames()
	{
		SharedStringList property_names = object->GetPropertyNames();
		for (size_t i = 0; i < this->Size(); i++)
		{
			std::string name = KList::IntToChars(i);
			property_names->push_back(new std::string(name));
		}

		return property_names;
	}

	PyObject* KPythonList::ToPython()
	{
		return this->object->ToPython();
	}

	bool KPythonList::Equals(SharedKObject other)
	{
		AutoPtr<KPythonList> pyOther = other.cast<KPythonList>();

		// This is not a Python object
		if (pyOther.isNull())
			return false;

		return pyOther->ToPython() == this->ToPython();
	}
}
