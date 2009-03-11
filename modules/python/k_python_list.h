/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PYTHON_LIST_H_
#define _K_PYTHON_LIST_H_

#include "python_module.h"

namespace kroll
{
	class KPythonList : public BoundList
	{
	public:
		KPythonList(PyObject *obj);
		virtual ~KPythonList();

		/**
		 * Append a value to this list. Value should be heap-allocated as
		 * implementors are allowed to keep a reference, if they increase the
		 * reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		void Append(SharedValue value);

		/**
		 * Get the length of this list.
		 */
		unsigned int Size();

		/**
		 * When an error occurs will throw an exception of type Value*.
		 * Return the value at the given index. The value is automatically
		 * reference counted and must be released.
		 * When an error occurs will throw an exception of type Value*.
		 */
		SharedValue At(unsigned int index);

		/**
		 * remove entry at index and returns true if found and removed
		 */
		bool Remove(unsigned int index);
		

		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference,
		 * if they increase the reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		void Set(const char *name, SharedValue value);

		/**
		 * return a named property. the returned value is automatically
		 * reference counted and you must release the reference when finished
		 * with the return value (even for Undefined and Null types).
		 * When an error occurs will throw an exception of type Value*.
		 */
		SharedValue Get(const char *name);

		/**
		 * Return a list of this object's property names.
		 */
		SharedStringList GetPropertyNames();

		PyObject* ToPython();

	protected:
		PyObject *list;
		SharedPtr<KPythonObject> object;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonList);
	};
}
#endif
