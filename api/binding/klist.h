/*
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
	/**
	 * A binding that represents a list§
	 */
	class KROLL_API KList : public KObject
	{
	public:
		KList() {}
		virtual ~KList() {}

		/**
		 * Append a value to this list
		 *  Errors will result in a thrown ValueException
		 */
		virtual void Append(SharedValue value) = 0;

		/**
		 * Get the length of this list.
		 */
		virtual unsigned int Size() = 0;

		/**
		 * @return the value at the given index.
		 * Errors will result in a thrown ValueException
		 */
		virtual SharedValue At(unsigned int index) = 0;

		/**
		 * Set the value at the given index. If the index is greater
		 * than the current list length, the list will be lengenthed
		 * by appending Value::Undefined;
		 * Errors will result in a thrown ValueException
		 */
		virtual void SetAt(unsigned int index, SharedValue value) = 0;

		/**
		 * Remove the list entry at the given index. Return true
		 * if found and removed.
		 * Errors will result in a thrown ValueException
		 */
		virtual bool Remove(unsigned int index) = 0;

		/**
		 * Set a property on this object to the given value
		 * Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, SharedValue value) = 0;

		/**
		 * @return the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name) = 0;

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/**
		 * @return a string representation of this object
		 */
		SharedString DisplayString(int levels=3);

		/**
		 * @return whether or not the passed-in string is an integer
		 */
		static bool IsInt(const char *name);

		/**
		 * @return the passed-in value as a string
		 */
		static std::string IntToChars(unsigned int value);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KList);
	};
}

#endif
