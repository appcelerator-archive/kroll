/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "scoped_lock.h"

namespace kroll
{
	ScopedLock::ScopedLock(Mutex *mutex) : mutex(mutex)
	{
		mutex->Lock();
	}

	ScopedLock::~ScopedLock()
	{
		mutex->Unlock();
	}
}


