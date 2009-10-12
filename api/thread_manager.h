/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_THREAD_MANAGER_H_
#define _KR_THREAD_MANAGER_H_

#ifdef OS_OSX
#define START_KROLL_THREAD NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
#define END_KROLL_THREAD [pool release];
#else
#define START_KROLL_THREAD
#define END_KROLL_THREAD
#endif

#endif
