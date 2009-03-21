/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_MUTEX_H_
#define _KR_MUTEX_H_

#include "base.h"

#ifdef OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

namespace kroll
{
	/**
	 * OS-independent mutex lock. this mutex can be recursively locked,
	 * however, make sure that for every call to Lock(), a corresponding
	 * Unlock() is called.
	 */
	class KROLL_API Mutex
	{
	public:

		Mutex();
		virtual ~Mutex();

		/**
		 * Lock this mutex
		 */
		void Lock();

		/**
		 * Unlock this mutex
		 */
		void Unlock();
	private:
	#ifdef OS_WIN32
		CRITICAL_SECTION mutex;
	#else
		pthread_mutex_t mutex;
	#endif
		DISALLOW_EVIL_CONSTRUCTORS(Mutex);
	};
}

#endif

