/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_THREAD_MANAGER_H_
#define _KR_THREAD_MANAGER_H_

#ifdef OS_OSX
#import <Foundation/Foundation.h>
#endif

namespace kroll
{
	class KROLL_API ThreadManager
	{
	public:
#ifdef OS_OSX
		ThreadManager()
		{
			this->pool = [[NSAutoreleasePool alloc] init];
		}

		~ThreadManager()
		{
			[pool release];
		}
	private:
		NSAutoreleasePool* pool;
#endif
	};
}
#endif
