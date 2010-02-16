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
		ProfiledBoundList(KListRef delegate);
		virtual ~ProfiledBoundList();

		// @see KList::Append
		virtual void Append(KValueRef value);
		// @see KList::Size
		virtual unsigned int Size();
		// @see KList::At
		virtual KValueRef At(unsigned int index);
		// @see KList::SetAt
		virtual void SetAt(unsigned int index, KValueRef value);
		// @see KList::Remove
		virtual bool Remove(unsigned int index);
		// @See KList::Set
		virtual void Set(const char *name, KValueRef value);
		// @see KList::Get
		virtual KValueRef Get(const char *name);
		// @see KList::GetPropertyNames
		virtual SharedStringList GetPropertyNames();

		bool HasProperty(const char* name);

		/**
		 * @return the delegate of this profiled bound object
		 */
		KListRef GetDelegate() { return list; }
		virtual void duplicate()
		{
			++count;
		}

		virtual void release()
		{
			int value = --count;
			if (value <= 0) {
				delete this;
			}
		}

		virtual int referenceCount() const
		{
			return count.value();
		}

	private:
		KListRef list;
		Poco::AtomicCounter count;

	};
}

#endif
