/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_LIST_H_
#define _KR_STATIC_BOUND_LIST_H_

namespace kroll
{
	/*
		Class: StaticBoundList
	*/
	class KROLL_API StaticBoundList : public KList
	{
	public:
		/*
			Constructor: StaticBoundList
		*/
		StaticBoundList();
		virtual ~StaticBoundList();
		static SharedBoundList FromStringVector(std::vector<std::string>&);

		/*
		 * Function: Append
		 *   Append a value to this list
		 *   Errors will result in a thrown ValueException
		 */
		virtual void Append(SharedValue value);

		/*
		 * Function: Size
		 *   Get the length of this list.
		 */
		virtual unsigned int Size();

		/*
		 * Function: At
		 *   Return the value at the given index.
		 *   Errors will result in a thrown ValueException
		 */
		virtual SharedValue At(unsigned int index);

		/*
		 * Function: Set
		 *   Set a property on this object to the given value
		 *   Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, SharedValue value);

		/*
		 * Function: Remove
		 *   Remove the list entry at the given index.
		 *   Errors will result in a thrown ValueException
		 * Returns: true if found and removed, false otherwise
		 */
		virtual bool Remove(unsigned int index);

		/*
		 * Function: SetAt
		 *   Set the value at the given index. If the index is greater
		 *   than the current list length, the list will be lengenthed
		 *   by appending Value::Undefined;
		 *   Errors will result in a thrown ValueException
		 */
		virtual void SetAt(unsigned int index, SharedValue value);

		/*
		 * Function: Get
		 *   Return the property with the given name or Value::Undefined
		 *   if the property is not found.
		 *   Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name);

		/*
		 * Function: GetPropertyNames
		 * Returns: a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames();

	protected:
		SharedPtr<StaticBoundObject> object;
		unsigned int length;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundList);
	};
}

#endif
