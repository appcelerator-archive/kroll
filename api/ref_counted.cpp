/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ref_counted.h"

namespace kroll
{
	RefCounted::RefCounted(KR_CALL_STACK_INFO) : 
#ifdef DEBUG_REFCOUNT
		filename(filename),
		linenumber(linenumber),
		func(func),
#endif
		count(1)
	{
		KR_CALL_STACK_DEBUG
	}

	RefCounted::~RefCounted()
	{
		if (count!=0)
		{
			std::cerr << "WARNING: Object: " << (void*)this << " freed with reference count == " << count << std::endl; 
		}
		KR_CALL_STACK_DEBUG
	}	

	RefCounted* RefCounted::AddReference(KR_CALL_STACK_INFO)
	{
		KR_CALL_STACK_DEBUG
		ScopedLock lock(&mutex);
		if (this->count==0)
		{
			throw "Invalid state.";
		}
		this->count++;
		return this;
	}

	void RefCounted::ReleaseReference(KR_CALL_STACK_INFO)
	{
		KR_CALL_STACK_DEBUG
		ScopedLock lock(&mutex);
		if (--count == 0)
		{
			delete this;
		}
	}

	const int RefCounted::ReferenceCount()
	{
		ScopedLock lock(&mutex);
		return count;
	}
}
