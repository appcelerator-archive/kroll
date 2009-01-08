/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _LOG_H
#define _LOG_H

#import <Foundation/Foundation.h>

namespace kroll
{
	extern void Log(NSString *);
	extern void SetupLog(int argc, const char *argv[], NSString *path);
	extern void CloseLog();
}

#ifdef DEBUG
   #define TRACE  NSLog
#else
   #define TRACE(s, ...)  if(s)TiLog([NSString stringWithFormat:@"[%@] <%p %@:(%d)> %@\n", [NSDate date], this, [[NSString stringWithUTF8String:__FILE__] lastPathComponent], __LINE__, [NSString stringWithFormat:(s), ##__VA_ARGS__]])
#endif

#endif