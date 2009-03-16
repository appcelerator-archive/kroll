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
	class KPythonList : public KList
	{
	public:
		KPythonList(PyObject *obj);
		virtual ~KPythonList();

		/*
		 * Function: Append
		 *   Append a value to this list
		 *   Errors will result in a thrown ValueException
		 */
		void Append(SharedValue value);

		/*
		 * Function: Size
		 *   Get the length of this list.
		 */
		unsigned int Size();

		/*
		 * Function: At
		 *   Return the value at the given index.
		 *   Errors will result in a thrown ValueException
		 */
		SharedValue At(unsigned int index);

		/*
		 * Function: SetAt
		 *   Set the value at the given index. If the index is greater
		 *   than the current list length, the list will be lengenthed
		 *   by appending Value::Undefined;
		 *   Errors will result in a thrown ValueException
		 */
		virtual void SetAt(unsigned int index, SharedValue value);

		/*
		 * Function: Remove
		 *   Remove the list entry at the given index. Return true
		 *   if found and removed. 
		 *   Errors will result in a thrown ValueException
		 */
		bool Remove(unsigned int index);

		/*
		 * Function: Set
		 *   Set a property on this object to the given value
		 *   Errors will result in a thrown ValueException
		 */
		void Set(const char *name, SharedValue value);

		/*
		 * Function: Get
		 *   Return the property with the given name or Value::Undefined
		 *   if the property is not found.
		 *   Errors will result in a thrown ValueException
		 */
		SharedValue Get(const char *name);

		/*
		 * Function: GetPropertyNames
		 * Returns: a list of this object's property names.
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

