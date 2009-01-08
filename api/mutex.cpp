/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "mutex.h"

namespace kroll
{
	Mutex::Mutex()
	{
	#ifdef OS_WIN32
		InitializeCriticalSection(&mutex);
	#else
		pthread_mutexattr_t mutexattr;
		int rc = pthread_mutexattr_init(&mutexattr);
		if (rc != 0) 
		{
			throw "couldn't create mutex - invalid mutex initialization";
		}

		rc = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
		if (rc != 0) 
		{
		  	pthread_mutexattr_destroy(&mutexattr);
		}

		rc = pthread_mutex_init(&mutex, &mutexattr);
		if (rc != 0) 
		{
		  	pthread_mutexattr_destroy(&mutexattr);
		}

		rc = pthread_mutexattr_destroy(&mutexattr);
		if (rc != 0) 
		{
		  	pthread_mutex_destroy(&mutex);
		}
	#endif	
	}

	Mutex::~Mutex()
	{
	#ifdef OS_WIN32
		DeleteCriticalSection(&mutex);
	#else
		pthread_mutex_destroy(&mutex);
	#endif	
	}

	void Mutex::Lock()
	{
	#ifdef OS_WIN32
		EnterCriticalSection(&mutex);
	#else
		pthread_mutex_lock(&mutex);
	#endif	
	}

	void Mutex::Unlock()
	{
	#ifdef OS_WIN32
		LeaveCriticalSection(&mutex);
	#else
		pthread_mutex_unlock(&mutex);
	#endif	
	}
}
