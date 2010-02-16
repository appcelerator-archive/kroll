/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_SCOPED_LOCK_H_
#define _KR_SCOPED_LOCK_H_

#include "base.h"
#include "mutex.h"

namespace kroll
{
	/**
	 * Utility class for locking a mutex on construction and unlocking
	 * on destruction.  This class is typically constructed as a stack
	 * variable so that the held mutex is locked when the stack is deleted.
	 */
	class KROLL_API ScopedLock
	{
	public:

		ScopedLock(Mutex *mutex);
		virtual ~ScopedLock();
	private:
		Mutex* mutex;
		DISALLOW_EVIL_CONSTRUCTORS(ScopedLock);
	};
}

#endif

