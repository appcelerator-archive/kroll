/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <vector>
#include <cstring>
#include <dlfcn.h>
#include <string>
#import <Cocoa/Cocoa.h>
#include "host.h"
#include <signal.h>

namespace kroll
{
	OSXHost::OSXHost(int _argc, const char **_argv) : Host(_argc,_argv)
	{
	}

	OSXHost::~OSXHost()
	{
	}

	const char* OSXHost::GetPlatform()
	{
		return "osx";
	}

	const char* OSXHost::GetModuleSuffix()
	{
		return "module.dylib";
	}

	bool OSXHost::Start()
	{
		NSApplication *app = [NSApplication sharedApplication];
		[app finishLaunching];
		Host::Start();
		return true;
	}

	bool OSXHost::RunLoop()
	{
		NSApplication *app = [NSApplication sharedApplication];
		// we pull out an event from the queue, blocking a little bit before returning
		try
		{
			@try
			{
				NSEvent *event = [app nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate dateWithTimeIntervalSinceNow:10.0] inMode:NSDefaultRunLoopMode dequeue:YES];
				if (event)
				{
					[app sendEvent:event];
					[app updateWindows];
				}
			}
			@catch(NSException *e)
			{
				std::cerr << "Caught NSException in main loop: " << [[e reason] UTF8String] << std::endl;
				KrollDumpStackTraceFromException(e);
			}
		}
		catch (std::exception &e)
		{
			std::cerr << "Caught exception in main loop: " << e.what() << std::endl;
			KrollDumpStackTrace();
		}
		catch (...)
		{
			std::cerr << "Caught unhandled exception in main loop: " << std::endl;
			KrollDumpStackTrace();
		}
		return true;
	}

	Module* OSXHost::CreateModule(std::string& path)
	{
		void* lib_handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!lib_handle)
		{
			std::cerr << "Error load module: " << path << ", error: " << dlerror() << std::endl;
			return 0;
		}

		// get the module factory
		ModuleCreator* create = (ModuleCreator*)dlsym(lib_handle, "CreateModule");
		if (!create)
		{
			std::cerr << "Error load create entry from module: " << path << std::endl;
			return 0;
		}

		std::string dir = FileUtils::GetDirectory(path);
		return create(this,dir.c_str());
	}
}

@interface KrollMainThreadCaller : NSObject
{
	SharedPtr<kroll::KMethod> *method;
	SharedPtr<kroll::Value> *result;
	SharedPtr<kroll::Value> *exception;
	ValueList *args;
	bool wait;
}
- (id)initWithKMethod:(SharedPtr<kroll::KMethod>)method args:(const ValueList*)args wait:(bool)wait;
- (void)call;
- (SharedPtr<kroll::Value>)getResult;
- (SharedPtr<kroll::Value>)getException;
@end

@implementation KrollMainThreadCaller
- (id)initWithKMethod:(SharedPtr<kroll::KMethod>)m args:(const ValueList*)a wait:(bool)w
{
	self = [super init];
	if (self)
	{
		method = new SharedPtr<kroll::KMethod>(m);
		args = new ValueList();
		for (size_t c=0;c<a->size();c++)
		{
			args->push_back(a->at(c));
		}
		wait = w;
		result = new SharedPtr<kroll::Value>();
		exception = new SharedPtr<kroll::Value>();
	}
	return self;
}
- (void)dealloc
{
	delete result;
	delete exception;
	delete method;
	delete args;
	[super dealloc];
}
- (SharedPtr<kroll::Value>)getResult
{
	return *result;
}
- (SharedPtr<kroll::Value>)getException
{
	return *exception;
}
- (void)call
{
	try
	{
		result->assign((*method)->Call(*args));
	}
	catch (ValueException &e)
	{
		exception->assign(e.GetValue());
	}
	catch (std::exception &e)
	{
		exception->assign(Value::NewString(e.what()));
	}
	catch (...)
	{
		exception->assign(Value::NewString("unhandled exception"));
	}
	if (!wait)
	{
		// on non-blocking we own ourselves and need to release
		[self release];
	}
}
@end

@interface NSThread(isMainThreadIsSafeReally)
+ (BOOL) isMainThread;
@end

@interface NSThread(isMainThreadLegacy)
+ (void) TiLegacyGetCurrentThread: (NSMutableData *) currentThreadData;
@end

@implementation NSThread(isMainThreadLegacy)
+ (void) TiLegacyGetCurrentThread: (NSMutableData *) currentThreadData;
{
	NSThread * currentThread = [NSThread currentThread];
	NSRange pointerRange = NSMakeRange(0,sizeof(NSThread *));
	[currentThreadData replaceBytesInRange:pointerRange withBytes:&currentThread]; //This copies the contents of currentThread, not its address.
}
@end

namespace kroll
{
	SharedValue OSXHost::InvokeMethodOnMainThread(SharedKMethod method,
	                                              const ValueList& args,
												  bool waitForCompletion)
	{
		// make sure to just invoke if we're already on the
		// main thread
		bool isMainThread;
		if ([NSThread respondsToSelector:@selector(isMainThread)]) 
		{
			isMainThread = [NSThread isMainThread];
		} 
		else 
		{
			NSMutableData * mainThreadData= [[NSMutableData alloc] initWithLength:sizeof(NSThread *)];
			[NSThread performSelectorOnMainThread:@selector(TiLegacyGetCurrentThread:) withObject:mainThreadData waitUntilDone:YES];
			NSThread * mainThread = *((id *)[mainThreadData bytes]);
			isMainThread = mainThread == [NSThread currentThread];
			[mainThreadData release];
		}

		if (isMainThread)
		{
			return method->Call(args);
		}
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		KrollMainThreadCaller *caller = [[KrollMainThreadCaller alloc] initWithKMethod:method args:&args wait:waitForCompletion];
		[caller performSelectorOnMainThread:@selector(call) withObject:nil waitUntilDone:waitForCompletion];
		if (!waitForCompletion)
		{
			[pool release];
			return Value::Null;
		}
		SharedValue exception = [caller getException];
		if (exception.isNull())
		{
			SharedValue result = [caller getResult];
			[caller release];
			[pool release];
			return result;
		}
		[caller release];
		[pool release];
		return exception;
	}
}

extern "C"
{
	int Execute(int argc,const char **argv)
	{
		kroll::Host *host = new kroll::OSXHost(argc,argv);
		return host->Run();
	}
}
