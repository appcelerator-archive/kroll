/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PYTHON_BOUND_LIST_H_
#define _PYTHON_BOUND_LIST_H_

#include "python_module.h"

namespace kroll
{
	class PythonBoundList : public BoundList
	{
	public:
		PythonBoundList(PyObject *obj);
		virtual ~PythonBoundList();
		PyObject* ToPython() const { Py_INCREF(object); return object; }

		/**
		 * Append a value to this list. Value should be heap-allocated as
		 * implementors are allowed to keep a reference, if they increase the
		 * reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		void Append(SharedPtr<Value> value);

		/**
		 * Get the length of this list.
		 */
		int Size();

		/**
		 * When an error occurs will throw an exception of type Value*.
		 * Return the value at the given index. The value is automatically
		 * reference counted and must be released.
		 * When an error occurs will throw an exception of type Value*.
		 */
		SharedPtr<Value> At(int index);

		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference,
		 * if they increase the reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		void Set(const char *name, SharedPtr<Value> value);

		/**
		 * return a named property. the returned value is automatically
		 * reference counted and you must release the reference when finished
		 * with the return value (even for Undefined and Null types).
		 * When an error occurs will throw an exception of type Value*.
		 */
		SharedPtr<Value> Get(const char *name);

		/**
		 * Return a list of this object's property names.
		 */
		SharedStringList GetPropertyNames();

	protected:
		PyObject *object;
        DISALLOW_EVIL_CONSTRUCTORS(PythonBoundList);
	};
}
#endif

