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
		if (!PyList_Check(list))
		{
			std::string msg("Invalid PyObject passed. Should be a Py_List");
			std::cerr << msg << std::endl;
			throw msg;
		}

		Py_INCREF(this->list);
	}

	KPythonList::~KPythonList()
	{
		Py_DECREF(this->list);
	}

	/**
	 * Append a value to this list. Value should be heap-allocated as
	 * implementors are allowed to keep a reference, if they increase the
	 * reference count.
	 * When an error occurs will throw an exception of type Value*.
	 */
	void KPythonList::Append(SharedValue value)
	{
		PyObject* py_value = PythonUtils::ToPyObject(value);
		PyList_Append(this->list, py_value);
	}

	/**
	 * Get the length of this list.
	 */
	unsigned int KPythonList::Size()
	{
		return PyList_Size(this->list);
	}

	/**
	 * remove the item and return true if removed
	 */
	bool KPythonList::Remove(unsigned int index)
	{
		PyObject* empty_list = PyList_New(0);
		PyList_SetSlice(this->list, index, index + 1, empty_list);
		Py_DECREF(empty_list);
		return true;
	}

	/**
	 * When an error occurs will throw an exception of type Value*.
	 * Return the value at the given index. The value is automatically
	 * reference counted and must be released.
	 * When an error occurs will throw an exception of type Value*.
	 */
	SharedValue KPythonList::At(unsigned int index)
	{
		PyObject *p = PyList_GetItem(this->list, index);
		if (p == NULL)
		{
			PyErr_Clear();
			return Value::Undefined;
		}

		SharedValue v = PythonUtils::ToKrollValue(p);
		Py_DECREF(p);
		return v;
	}

	/**
	 * Set a property on this object to the given value. Value should be
	 * heap-allocated as implementors are allowed to keep a reference,
	 * if they increase the reference count.
	 * When an error occurs will throw an exception of type Value*.
	 */
	void KPythonList::Set(const char *name, SharedValue value)
	{
		// Check for integer value as name
		if (BoundList::IsInt(name))
		{
			int index = atoi(name);
			while (index >= (int) this->Size())
			{
				// now we need to create entries
				// between current size and new size
				// and make the entries undefined.
				this->Append(Value::Undefined);
			}

			PyObject* py_value = PythonUtils::ToPyObject(value);
			PyList_SetItem(this->list, index, py_value);
		}
		else
		{
			this->object->Set(name, value);
		}
	}

	/**
	 * return a named property. the returned value is automatically
	 * reference counted and you must release the reference when finished
	 * with the return value (even for Undefined and Null types).
	 * When an error occurs will throw an exception of type Value*.
	 */
	SharedValue KPythonList::Get(const char *name)
	{
		if (std::string(name) == std::string("length"))
		{
			return Value::NewInt(this->Size());
		}
		else if (BoundList::IsInt(name))
		{
			unsigned int index = (unsigned int) atoi(name);
			return this->At(index);
		}
		else
		{
			return object->Get(name);
		}
	}

	/**
	 * Return a list of this object's property names.
	 */
	SharedStringList KPythonList::GetPropertyNames()
	{
		SharedStringList property_names = object->GetPropertyNames();
		property_names->push_back(new std::string("length"));
		for (size_t i = 0; i < this->Size(); i++)
		{
			std::string name = BoundList::IntToChars(i);
			property_names->push_back(new std::string(name));
		}

		return property_names;
	}

	PyObject* KPythonList::ToPython()
	{
		return this->object->ToPython();
	}
}
