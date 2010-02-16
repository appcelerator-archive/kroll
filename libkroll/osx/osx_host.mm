/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../kroll.h"

#import <iostream>
#import <vector>
#import <dlfcn.h>
#import <string>
#import <signal.h>
#import <Cocoa/Cocoa.h>
#import <openssl/crypto.h>
#import <Poco/Mutex.h>

#define MAIN_THREAD_JOB_EVENT_SUBTYPE 6666

@interface KrollMainThreadCaller : NSObject
{
}
- (void)signalMainThreadJob;
@end
@implementation KrollMainThreadCaller
- (void)signalMainThreadJob
{
	Host::GetInstance()->RunMainThreadJobs();
}
@end

namespace kroll
{
	static NSThread* mainThread;
	static Poco::Mutex* cryptoMutexes = 0;
	static KrollMainThreadCaller* mainThreadCaller = 0;

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

	void Host::Initialize(int argc, const char **argv)
	{
		if (!cryptoMutexes)
		{
			cryptoMutexes = new Poco::Mutex[CRYPTO_num_locks()];
			CRYPTO_set_id_callback(CryptoThreadIdCallback);
			CRYPTO_set_locking_callback(CryptoLockingCallback);
		}

		if (!mainThreadCaller)
			mainThreadCaller = [[KrollMainThreadCaller alloc] init];

		mainThread = [NSThread currentThread];
	}

	Host::~Host()
	{
		if (!cryptoMutexes)
			return;

		delete [] cryptoMutexes;
		cryptoMutexes = 0;
	}

	void Host::WaitForDebugger()
	{
		printf("Waiting for debugger (Press Any Key to Continue pid=%i)...\n", getpid());
		getchar();
	}

	bool Host::IsMainThread()
	{
		return [NSThread currentThread] == mainThread;
	}

	bool Host::RunLoop()
	{
		NSApplication* application = [NSApplication sharedApplication];

		string origPath(EnvironmentUtils::Get("KR_ORIG_DYLD_FRAMEWORK_PATH"));
		EnvironmentUtils::Set("DYLD_FRAMEWORK_PATH", origPath);
		origPath = EnvironmentUtils::Get("KR_ORIG_DYLD_LIBRARY_PATH");
		EnvironmentUtils::Set("DYLD_LIBRARY_PATH", origPath);
		[application finishLaunching];

		[application run];
		return false;
	}

	void Host::SignalNewMainThreadJob()
	{
		[mainThreadCaller
			performSelectorOnMainThread:@selector(signalMainThreadJob)
			withObject:nil
			waitUntilDone:false];
	}

	void Host::ExitImpl(int exitcode)
	{
		// Stop the event loop, but don't terinate the application.
		// The host still needs to clean up everything.
		[[NSApplication sharedApplication] stop:nil];

		// NSApplication stop: doesn't stop the main loop until an event
		// has been processed, so push an application specific event through.
		[[NSApplication sharedApplication] postEvent:[NSEvent
			otherEventWithType:NSApplicationDefined
			location:NSZeroPoint
			modifierFlags:0
			timestamp:[[NSDate date] timeIntervalSinceNow]
			windowNumber:0
			context:nil
			subtype:MAIN_THREAD_JOB_EVENT_SUBTYPE
			data1:0
			data2:0]
			atStart:YES];
	}

	Module* Host::CreateModule(std::string& path)
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
}
