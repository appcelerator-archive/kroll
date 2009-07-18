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
		ProfiledBoundList(SharedKList delegate);
		virtual ~ProfiledBoundList();

		// @see KList::Append
		virtual void Append(SharedValue value);
		// @see KList::Size
		virtual unsigned int Size();
		// @see KList::At
		virtual SharedValue At(unsigned int index);
		// @see KList::SetAt
		virtual void SetAt(unsigned int index, SharedValue value);
		// @see KList::Remove
		virtual bool Remove(unsigned int index);
		// @See KList::Set
		virtual void Set(const char *name, SharedValue value);
		// @see KList::Get
		virtual SharedValue Get(const char *name);
		// @see KList::GetPropertyNames
		virtual SharedStringList GetPropertyNames();

		bool HasProperty(const char* name);

		/**
		 * @return the delegate of this profiled bound object
		 */
		SharedKList GetDelegate() { return list; }
		virtual void duplicate()
		{
			referenceCount++;
		}

		virtual void release()
		{
			referenceCount--;
			if (referenceCount.value() <= 0) {
				delete this;
			}
		}

	private:
		SharedKList list;
		Poco::AtomicCounter referenceCount;

	};
}

#endif
