/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_BOUND_LIST_H_
#define _KR_PROFILED_BOUND_LIST_H_

namespace kroll
{
	/**
	 * The ProfiledBoundList is a wrapped KList that does profiling
	 */
	class ProfiledBoundList : public ProfiledBoundObject, public KList
	{
	public:
		ProfiledBoundList(std::string name, SharedKList delegate, Poco::FileOutputStream *stream);
		virtual ~ProfiledBoundList();
	private:
		SharedKList list;

	public:

		/**
		 * Append a value to this list
		 *  Errors will result in a thrown ValueException
		 */
		virtual void Append(SharedValue value);

		/**
		 * Get the length of this list.
		 */
		virtual unsigned int Size();

		/**
		 * @return the value at the given index.
		 * Errors will result in a thrown ValueException
		 */
		virtual SharedValue At(unsigned int index);

		/**
		 * Set the value at the given index. If the index is greater
		 * than the current list length, the list will be lengenthed
		 * by appending Value::Undefined;
		 * Errors will result in a thrown ValueException
		 */
		virtual void SetAt(unsigned int index, SharedValue value);

		/**
		 * Remove the list entry at the given index. Return true
		 * if found and removed.
		 * Errors will result in a thrown ValueException
		 */
		virtual bool Remove(unsigned int index);

		/**
		 * Set a property on this object to the given value
		 * Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, SharedValue value);

		/**
		 * @return the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name);

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames();

		bool HasProperty(const char* name);

		/**
		 * @return the delegate of this profiled bound object
		 */
		SharedKList GetDelegate() { return list; }

	};
}

#endif
