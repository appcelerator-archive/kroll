/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_REF_COUNTED_H_
#define _KR_REF_COUNTED_H_

#include "base.h"
#include "mutex.h"
#include "scoped_lock.h"

namespace kroll
{
	/**
	 * reference counted base class. this object should be created
	 * (reference count is initialized to 1 on construction) and never
	 * directly deleted.  to delete the object, call ReleaseReference()
	 * to release your reference.  once all references have been released
	 * (reference count = 0), the object will be automatically deleted.
	 * the methods on this object are thread-safe.
	 */
	class KROLL_API RefCounted 
	{
	public:
		RefCounted();
	protected:
		virtual ~RefCounted();
	public:
		RefCounted* AddReference(
	#ifdef DEBUG_REFCOUNT
			const char* fn, int ln
	#endif
		);
		void ReleaseReference(
	#ifdef DEBUG_REFCOUNT
			const char* fn, int ln
	#endif
		);
		const int ReferenceCount();
	private:
		int count;
		Mutex mutex;
		DISALLOW_EVIL_CONSTRUCTORS(RefCounted);	
	};
}

#endif

