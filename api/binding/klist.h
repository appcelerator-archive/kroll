
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_LIST_H_
#define _KR_BOUND_LIST_H_

#include <string>
#include <cctype>

namespace kroll
{
	/*
		Class: KList
	*/
	class KROLL_API KList : public KObject
	{
	public:
		KList() {}
		virtual ~KList() {}

		/*
		 * Function: Append
		 *   Append a value to this list
		 *   Errors will result in a thrown ValueException
		 */
		virtual void Append(SharedValue value) = 0;

		/*
		 * Function: Size
		 *   Get the length of this list.
		 */
		virtual unsigned int Size() = 0;

		/*
		 * Function: At
		 *   Return the value at the given index.
		 *   Errors will result in a thrown ValueException
		 */
		virtual SharedValue At(unsigned int index) = 0;

		/*
		 * Function: SetAt
		 *   Set the value at the given index. If the index is greater
		 *   than the current list length, the list will be lengenthed
		 *   by appending Value::Undefined;
		 *   Errors will result in a thrown ValueException
		 */
		virtual void SetAt(unsigned int index, SharedValue value) = 0;

		/*
		 * Function: Remove
		 *   Remove the list entry at the given index. Return true
		 *   if found and removed. 
		 *   Errors will result in a thrown ValueException
		 */
		virtual bool Remove(unsigned int index) = 0;

		/*
		 * Function: Set
		 *   Set a property on this object to the given value
		 *   Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, SharedValue value) = 0;

		/*
		 * Function: Get
		 *   Return the property with the given name or Value::Undefined
		 *   if the property is not found.
		 *   Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name) = 0;

		/*
		 * Function: GetPropertyNames
		 * Returns: a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/*
		 * Function: DisplayString
		 * Returns: a string representation of this object
		 */
		SharedString DisplayString(int levels=3);

		static bool IsInt(const char *name);
		static std::string IntToChars(unsigned int value);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KList);
	};
}

#endif
