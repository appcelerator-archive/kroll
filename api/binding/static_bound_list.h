/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_LIST_H_
#define _KR_STATIC_BOUND_LIST_H_

namespace kroll
{

	class KROLL_API StaticBoundList : public KList
	{
	public:

		StaticBoundList(const char *type = "StaticBoundList");
		virtual ~StaticBoundList();
		static KListRef FromStringVector(std::vector<std::string>&);

		/**
		 * Append a value to this list
		 * Errors will result in a thrown ValueException
		 */
		virtual void Append(KValueRef value);

		/**
		 * Get the length of this list.
		 */
		virtual unsigned int Size();

		/**
		 * @return the value at the given index.
		 * Errors will result in a thrown ValueException
		 */
		virtual KValueRef At(unsigned int index);

		/**
		 * Set a property on this object to the given value
		 * Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, KValueRef value);

		/**
		 * Remove the list entry at the given index.
		 * Errors will result in a thrown ValueException
		 * @return true if found and removed, false otherwise
		 */
		virtual bool Remove(unsigned int index);

		/**
		 * Set the value at the given index. If the index is greater
		 * than the current list length, the list will be lengenthed
		 * by appending Value::Undefined;
		 * Errors will result in a thrown ValueException
		 */
		virtual void SetAt(unsigned int index, KValueRef value);

		/**
		 * @return the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual KValueRef Get(const char *name);

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames();

	protected:
		AutoPtr<StaticBoundObject> object;
		unsigned int length;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundList);
	};
}

#endif
