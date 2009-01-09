/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "pythonlist.h"
#include "pythontypes.h"
#include <vector>

namespace kroll
{
	PythonList::PythonList(PyObject *obj) : object(obj)
	{
		Value *length = new Value((int)PyList_Size(this->object));
		this->Set("length",length);
		KR_DECREF(length);
		Py_INCREF(this->object);
	}

	PythonList::~PythonList()
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
	void PythonList::Append(Value* value)
	{
		PyList_Append(this->object,ValueToPythonValue(value));
		KR_DECREF(value);
	}

	/**
	 * Get the length of this list.
	 */
	int PythonList::Size()
	{
		return PyList_Size(this->object);
	}

	/**
	 * When an error occurs will throw an exception of type Value*.
	 * Return the value at the given index. The value is automatically
	 * reference counted and must be released.
	 * When an error occurs will throw an exception of type Value*.
	 */
	Value* PythonList::At(int index)
	{
		PyObject *p = PyList_GET_ITEM(this->object,index);
		if (Py_None == p)
		{
			Py_DECREF(p);
			return Value::Undefined();
		}
		Value *v = PythonValueToValue(p,NULL);
		Py_DECREF(p);
		return v;
	}

	/**
	 * Set a property on this object to the given value. Value should be
	 * heap-allocated as implementors are allowed to keep a reference,
	 * if they increase the reference count.
	 * When an error occurs will throw an exception of type Value*.
	 */
	void PythonList::Set(const char *name, Value* value)
	{
		// check for integer value as name
		if (this->IsNumber(name))
		{
			// TODO: checkrange
			Value* current = this->At(atoi(name));
			if (current->IsUndefined())
			{
				// release undefined
				KR_DECREF(current);
				// accessing via prop[0] but where we don't yet have an entry - do an append
				this->Append(value);
			}
			else
			{
				current->Set(value);
			}
		}
		else
		{
			// set a named property
			int result = PyObject_SetAttrString(this->object,(char*)name,ValueToPythonValue(value));

			PyObject *exception = PyErr_Occurred();
			if (result == -1 && exception != NULL)
			{
				PyErr_Clear();
				throw PythonValueToValue(exception, NULL);
			}
		}
	}

	/**
	 * return a named property. the returned value is automatically
	 * reference counted and you must release the reference when finished
	 * with the return value (even for Undefined and Null types).
	 * When an error occurs will throw an exception of type Value*.
	 */
	Value* PythonList::Get(const char *name)
	{
		
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
			PyErr_Clear();
			Py_XDECREF(response);
			throw PythonValueToValue(exception, NULL);
		}

		Value* returnValue = PythonValueToValue(response,name);
		Py_DECREF(response);
		return returnValue;
	}

	/**
	 * Return a list of this object's property names.
	 */
	std::vector<std::string> PythonList::GetPropertyNames()
	{
		std::vector<std::string> names;
		PyObject *props = PyObject_Dir(this->object);
		
		if (props == NULL)
		{
			Py_DECREF(props);
			return names;
		}

		PyObject *iterator = PyObject_GetIter(props);
		PyObject *item;

		if (iterator == NULL)
		{
			return names;
		}
		
		while ((item = PyIter_Next(iterator))) 
		{
			std::string name = PythonStringToString(item);
			names.push_back(name);
			Py_DECREF(item);
		}
		
		Py_DECREF(iterator);
		Py_DECREF(props);
		return names;
	}
}
