/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <api/kroll.h>
#include <host.h>

#if defined(OS_OSX)
#import <Cocoa/Cocoa.h>
#endif

using namespace kroll;

#if defined(OS_WIN32) && !defined(WIN32_CONSOLE)
#include <windows.h>
int WinMain(HINSTANCE, HINSTANCE, LPSTR command_line, int)
#else
int main(int argc, const char* argv[])
#endif
{
#if defined(OS_OSX)
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
#endif

	printf("Starting up the Kroll kernel...\n");

	//create our host instance
#if defined(OS_OSX)
	Host *host = new OSXHost(argc,argv);
#elif defined(OS_LINUX)
	Host *host = new LinuxHost(argc,argv);
#elif defined(OS_WIN32)
	Host *host = new Win32Host(::GetModuleHandle(NULL), __argc,(const char**)__argv);
#endif

	printf("Created host, booting...\n");

	int rc = host->Run();

#if defined(OS_OSX)
	[pool release];
#endif

	printf("Exiting Kroll, bye...\n");
	return rc;
}
