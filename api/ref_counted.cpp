/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ref_counted.h"

namespace kroll
{
	RefCounted::RefCounted() : count(1)
	{
	#ifdef DEBUG_REFCOUNT
		std::cout << "Created: " << (void*)this << " (" << ")"<< std::endl;
	#endif
	}

	RefCounted::~RefCounted()
	{
	#ifdef DEBUG_REFCOUNT
		std::cout << "Destroying: " << (void*)this << std::endl;
	#endif
		if (count!=0)
		{
			std::cerr << "WARNING: Object: " << (void*)this << " freed with reference count == " << count << std::endl; 
		}
	}	

	RefCounted* RefCounted::AddReference(
	#ifdef DEBUG_REFCOUNT
	 	const char *fn, int ln
	#endif
	)
	{
	#ifdef DEBUG_REFCOUNT
		std::cout << "AddReference: " << (void*)this << " from " << fn << ":" << ln << std::endl;
	#endif
		ScopedLock lock(&mutex);
		if (this->count==0)
		{
			throw "Invalid state.";
		}
		this->count++;
		return this;
	}

	void RefCounted::ReleaseReference(
	#ifdef DEBUG_REFCOUNT
	 	const char *fn, int ln
	#endif
	)
	{
	#ifdef DEBUG_REFCOUNT
		std::cout << "ReleaseReference: " << (void*)this << " from " << fn << ":" << ln << std::endl;
	#endif
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
