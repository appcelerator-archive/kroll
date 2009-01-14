/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_bound_list.h"

namespace kroll
{
	PythonBoundList::PythonBoundList(PyObject *obj) : object(obj)
	{
		if (!PyList_Check(obj))
		{
			std::string msg("Invalid PyObject passed. Should be a Py_List");
			std::cerr << msg << std::endl;
			throw msg;
		}

		Py_INCREF(this->object);
	}

	PythonBoundList::~PythonBoundList()
	{
		Py_DECREF(this->object);
		this->object = NULL;
	}

	/**
	 * Append a value to this list. Value should be heap-allocated as
	 * implementors are allowed to keep a reference, if they increase the
	 * reference count.
	 * When an error occurs will throw an exception of type Value*.
	 */
	void PythonBoundList::Append(Value* value)
	{
		PyList_Append(this->object,ValueToPythonBoundObject(value));
		KR_DECREF(value);
	}

	/**
	 * Get the length of this list.
	 */
	int PythonBoundList::Size()
	{
		return PyList_Size(this->object);
	}

	/**
	 * When an error occurs will throw an exception of type Value*.
	 * Return the value at the given index. The value is automatically
	 * reference counted and must be released.
	 * When an error occurs will throw an exception of type Value*.
	 */
	Value* PythonBoundList::At(int index)
	{
		PyObject *p = PyList_GET_ITEM(this->object,index);
		if (Py_None == p)
		{
			Py_DECREF(p);
			return Value::Undefined();
		}
		Value *v = PythonBoundObjectToValue(p,NULL);
		Py_DECREF(p);
		return v;
	}

	/**
	 * Set a property on this object to the given value. Value should be
	 * heap-allocated as implementors are allowed to keep a reference,
	 * if they increase the reference count.
	 * When an error occurs will throw an exception of type Value*.
	 */
	void PythonBoundList::Set(const char *name, Value* value)
	{
		// check for integer value as name
		if (this->IsNumber(name))
		{
			int val = atoi(name);
			if (val >= this->Size())
			{
				// now we need to create entries
				// between current size and new size
				// and make the entries null
				for (int c = this->Size(); c <= val; c++)
				{
					this->Append(NULL);
				}
			}

			Value* current = this->At(val);
			current->Set(value);
		}
		else
		{
			// set a named property
			PyObject* py = ValueToPythonBoundObject(value);
			int result = PyObject_SetAttrString(this->object,(char*)name,py);
			Py_DECREF(py);

			PyObject *exception = PyErr_Occurred();
			if (result == -1 && exception != NULL)
			{
				ThrowPythonException();
			}
		}
	}

	/**
	 * return a named property. the returned value is automatically
	 * reference counted and you must release the reference when finished
	 * with the return value (even for Undefined and Null types).
	 * When an error occurs will throw an exception of type Value*.
	 */
	Value* PythonBoundList::Get(const char *name)
	{
		if (std::string(name) == std::string("length"))
		{
			return new Value(this->Size());
		}
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

		Value* returnValue = PythonBoundObjectToValue(response,name);
		Py_DECREF(response);
		return returnValue;
	}

	/**
	 * Return a list of this object's property names.
	 */
	void PythonBoundList::GetPropertyNames(std::vector<const char *> *property_names)
	{
		property_names->push_back("length");

		PyObject *props = PyObject_Dir(this->object);

		if (props == NULL)
		{
			Py_DECREF(props);
			return;
		}

		PyObject *iterator = PyObject_GetIter(props);
		PyObject *item;

		if (iterator == NULL)
		{
			return;
		}

		while ((item = PyIter_Next(iterator)))
		{
			property_names->push_back(PythonStringToString(item));
			Py_DECREF(item);
		}

		Py_DECREF(iterator);
		Py_DECREF(props);
	}
}
