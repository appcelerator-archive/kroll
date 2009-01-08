/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import "log.h"

NSFileHandle *Logger;

namespace kroll
{
	//
	// this method is called by the TRACE macro and shouldn't be (generally)
	// called directly
	//
	void Log(NSString *message)
	{
	   if (tiLogger)
	   {
	      NSData *data = [message dataUsingEncoding: NSUTF8StringEncoding];
	      [tiLogger writeData:data];
	   }
	}

	//
	// setup the log stream
	//
	void SetupLog(int argc, const char *argv[], NSString *path)
	{
	   NSLog(@"setup log called with: %@",path);
	
	   for (int c=0;c<argc;c++)
	   {
	      const char *e = argv[c];
	      if (strstr(e,"--console"))
	      {
	         // we want to log to stdout in the case of console
	         tiLogger = [NSFileHandle fileHandleWithStandardOutput];
	         return;
	      }
	   }
	   // ensure that the file is available 
	   [[NSFileManager defaultManager] createFileAtPath:path contents:@"" attributes:nil];
	   // open it
	   Logger = [NSFileHandle fileHandleForWritingAtPath:path];
	}

	//
	// close the log stream
	// 
	void CloseLog()
	{
	   [Logger synchronizeFile];
	   [Logger closeFile];
	   [Logger release];
	}
}
