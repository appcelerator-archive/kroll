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

	void OSXHost::Exit(int exitcode)
	{
		// We're going to post our event to our event queue to cause him
		// to wake up (since he'll block waiting for pending events)
		NSEvent *event = [NSEvent
			otherEventWithType:NSApplicationDefined
			location:NSZeroPoint modifierFlags:0
			timestamp:[[NSDate date] timeIntervalSinceNow]
			windowNumber:0 
			context:nil 
			subtype:1022 
			data1:exitCode
			data2:0];
		NSApplication *app = [NSApplication sharedApplication];
		[app postEvent:event atStart:YES];
		Host::Exit(exitCode);
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
		// Number of iterations before we yield back control
		// to the main loop driver in the superclass
		static int iterations = 10;

		// Number of seconds we want to block waiting on a 
		// pending event to be visible in the queue
		// 0.5 seems to be the most optimal based on testing 
		// on Leopard and using some basic animated JS tests 
		// like John Resig's processing.js demos. 0.5 seems to
		// keep the CPU at effectively 0.0% when idle and seems
		// to give us very good frame rate and CPU during animation
		static double waitTime = 0.5;

		// Since we call this method a lot, set the app in a static var
		static NSApplication *app = [NSApplication sharedApplication];

		// Create a pool to sweep memory throught the loop
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		for (int c = 0; c < iterations; c++)
		{
			// we pull out an event from the queue, blocking a little bit before returning
			@try
			{
				NSEvent *event = [app nextEventMatchingMask:NSAnyEventMask 
					untilDate:[NSDate dateWithTimeIntervalSinceNow:waitTime]
					inMode:NSDefaultRunLoopMode dequeue:YES];

				if (event)
				{
					NSEventType type = [event type];

					// This is our custom-defined stop event
					if (type == NSApplicationDefined &&
						[event subtype] == 1022 && [event data1] == 0)
					{
						[pool release];
						return false;
					}
					else
					{
						[app sendEvent:event];
					}

					[app updateWindows];
				} }
			@catch (NSException *e)
			{
				static Logger* logger = Logger::Get("Host");
				logger->Error("Caught NSException in main loop: %s",[[e reason] UTF8String]);
				KrollDumpStackTraceFromException(e);
			}
		}
		[pool release];
		return true;
	}

	Module* OSXHost::CreateModule(std::string& path)
	{
		void* handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!handle)
		{
			throw ValueException::FromFormat("Error loading module (%s): %s\n", path.c_str(), dlerror());
		}

		// Get the module factory symbol.
		ModuleCreator* create = (ModuleCreator*) dlsym(handle, "CreateModule");
		if (!create)
		{
			throw ValueException::FromFormat("Cannot load CreateModule symbol from module (%s): %s\n", path.c_str(), dlerror());
		}

		std::string dir(FileUtils::GetDirectory(path));
		return create(this, dir.c_str());
	}
}

@interface KrollMainThreadCaller : NSObject
{
	KMethodRef *method;
	KValueRef *result;
	KValueRef *exception;
	ValueList *args;
	bool wait;
}
- (id)initWithKMethod:(KMethodRef)method args:(const ValueList*)args wait:(bool)wait;
- (void)call;
- (KValueRef)getResult;
- (KValueRef)getException;
@end

@implementation KrollMainThreadCaller
- (id)initWithKMethod:(KMethodRef)m args:(const ValueList*)a wait:(bool)w
{
	self = [super init];
	if (self)
	{
		method = new KMethodRef(m);
		args = new ValueList();
		for (size_t c=0;c<a->size();c++)
		{
			args->push_back(a->at(c));
		}
		wait = w;
		result = new KValueRef();
		exception = new KValueRef();
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
- (KValueRef)getResult
{
	return *result;
}
- (KValueRef)getException
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
	KValueRef OSXHost::InvokeMethodOnMainThread(
		KMethodRef method,
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

		if (isMainThread && waitForCompletion)
		{
			return method->Call(args);
		}
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		KrollMainThreadCaller *caller = [[KrollMainThreadCaller alloc] initWithKMethod:method args:&args wait:waitForCompletion];
		[caller performSelectorOnMainThread:@selector(call) withObject:nil waitUntilDone:waitForCompletion];
		if (!waitForCompletion)
		{
			[pool release];
			return Value::Undefined;
		}
		KValueRef exception = [caller getException];
		if (exception.isNull())
		{
			KValueRef result = [caller getResult];
			[caller release];
			[pool release];
			return result;
		}
		[caller release];
		[pool release];
		throw ValueException(exception);
		return Value::Undefined;
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
