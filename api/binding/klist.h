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
		KList(const char *type = "KList") : KObject(type) {}
		virtual ~KList() {}

		/**
		 * Append a value to this list
		 *  Errors will result in a thrown ValueException
		 */
		virtual void Append(KValueRef value) = 0;

		/**
		 * Get the length of this list.
		 */
		virtual unsigned int Size() = 0;

		/**
		 * @return the value at the given index.
		 * Errors will result in a thrown ValueException
		 */
		virtual KValueRef At(unsigned int index) = 0;

		/**
		 * Set the value at the given index. If the index is greater
		 * than the current list length, the list will be lengenthed
		 * by appending Value::Undefined;
		 * Errors will result in a thrown ValueException
		 */
		virtual void SetAt(unsigned int index, KValueRef value) = 0;

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
		virtual void Set(const char *name, KValueRef value) = 0;

		/**
		 * @return the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual KValueRef Get(const char *name) = 0;

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/**
		 * Modify this size of this list based. If the size is larger
		 * than the current length the list will be padded with Undefined.
		 * If the size is smaller than the current list the list will be
		 * truncated. If the size is invalid, nothing will happen.
		 * @arg size the new size of the list.
		 */
		void ResizeTo(unsigned int size);

		/**
		 * @return a string representation of this object
		 */
		SharedString DisplayString(int levels=3);

		/**
		 * @return whether or not the passed-in string is an integer
		 */
		static bool IsInt(const std::string& name);

		/**
		 * @return the passed-in value as a string
		 */
		static std::string IntToChars(unsigned int value);

		/**
		 * Return the unwrapped version of this object
		 */
		static KListRef Unwrap(KListRef);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KList);
	};
}

#endif
