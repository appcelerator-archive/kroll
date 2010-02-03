/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "host.h"

#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <string>
#include <signal.h>
#import <Cocoa/Cocoa.h>
#include <openssl/crypto.h>
#include <Poco/Mutex.h>

@interface KrollMainThreadCaller : NSObject
{
	MainThreadJob* job;
}
- (id)initWithJob:(MainThreadJob*)jobIn;
- (void)execute;
@end

@implementation KrollMainThreadCaller
- (id)initWithJob:(MainThreadJob*)jobIn
{
	self = [super init];
	if (self)
	{
		job = jobIn;
	}
	return self;
}
- (void)dealloc
{
	delete job;
	[super dealloc];
}
- (MainThreadJob*)job
{
	return job;
}
- (void)execute
{
	job->Execute();

	// When executing asynchronously, we need to clean ourselves up.
	if (!job->ShouldWaitForCompletion())
	{
		job->PrintException();
		[self release];
	}
}
@end

namespace kroll
{
	static NSThread* mainThread;
	static Poco::Mutex* cryptoMutexes = 0;

	static void CryptoLockingCallback(int mode, int n, const char* file, int line)
	{
		if (mode & CRYPTO_LOCK)
			cryptoMutexes[n].lock();
		else
			cryptoMutexes[n].unlock();
	}

	static unsigned long CryptoThreadIdCallback(void)
	{
		return ((unsigned long) pthread_self());
	}

	static void InitializeCryptoMutexes()
	{
		if (!cryptoMutexes)
		{
			cryptoMutexes = new Poco::Mutex[CRYPTO_num_locks()];
			CRYPTO_set_id_callback(CryptoThreadIdCallback);
			CRYPTO_set_locking_callback(CryptoLockingCallback);
		}
	}

	static void CleanupCryptoMutexes()
	{
		delete [] cryptoMutexes;
		cryptoMutexes = 0;
	}

	OSXHost::OSXHost(int _argc, const char **_argv) : Host(_argc,_argv)
	{
		InitializeCryptoMutexes();
		mainThread = [NSThread currentThread];
	}

	OSXHost::~OSXHost()
	{
		CleanupCryptoMutexes();
	}

	bool OSXHost::IsMainThread()
	{
		return [NSThread currentThread] == mainThread;
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
		Host::Exit(exitCode);

		// Check to see if the event handler cancelled this event.
		if (!this->exiting)
			return;

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
	}

	bool OSXHost::Start()
	{
		string origPath(EnvironmentUtils::Get("KR_ORIG_DYLD_FRAMEWORK_PATH"));
		EnvironmentUtils::Set("DYLD_FRAMEWORK_PATH", origPath);
		origPath = EnvironmentUtils::Get("KR_ORIG_DYLD_LIBRARY_PATH");
		EnvironmentUtils::Set("DYLD_LIBRARY_PATH", origPath);

		[[NSApplication sharedApplication] finishLaunching];
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
			throw ValueException::FromFormat("Error loading module (%s): %s\n",
				path.c_str(), dlerror());
		}

		// Get the module factory symbol.
		ModuleCreator* create = (ModuleCreator*) dlsym(handle, "CreateModule");
		if (!create)
		{
			throw ValueException::FromFormat("Cannot load CreateModule symbol from module "
				"(%s): %s\n", path.c_str(), dlerror());
		}

		std::string dir(FileUtils::GetDirectory(path));
		return create(this, dir.c_str());
	}

	KValueRef OSXHost::RunOnMainThread(KMethodRef method, KObjectRef thisObject,
		const ValueList& args, bool waitForCompletion)
	{
		if (this->IsMainThread() && waitForCompletion)
		{
			return method->Call(args);
		}

		MainThreadJob* job = new MainThreadJob(method, thisObject,
			args, waitForCompletion);
		KrollMainThreadCaller* caller = 
			[[KrollMainThreadCaller alloc] initWithJob:job];
		[caller performSelectorOnMainThread:@selector(execute)
			withObject:nil waitUntilDone:waitForCompletion];

		if (!waitForCompletion)
		{
			// The job will release itself.
			return Value::Undefined;
		}

		KValueRef result(job->GetResult());
		ValueException exception(job->GetException());
		[caller release];

		if (!result.isNull())
			return result;
		else
			throw exception;
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
