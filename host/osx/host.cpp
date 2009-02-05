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

namespace kroll
{
	OSXHost::OSXHost(int _argc, const char **_argv) : Host(_argc,_argv)
	{
		//FIXME - push this up
		char *p = getenv("KR_MODULES");
		if (p)
		{
			FileUtils::Tokenize(p, this->module_paths, ":");
		}
		
		[NSApplication sharedApplication];
	}

	OSXHost::~OSXHost()
	{
	}
	
	bool OSXHost::Start()
	{
		Host::Start();
		NSApplication *app = [NSApplication sharedApplication];
		[app finishLaunching];
		return true;
	}

	bool OSXHost::RunLoop()
	{
		NSApplication *app = [NSApplication sharedApplication];
		// we pull out an event from the queue, blocking a little bit before returning
		NSEvent *event = [app nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate dateWithTimeIntervalSinceNow:1] inMode:NSDefaultRunLoopMode dequeue:YES];
		if (event) 
		{
			[app sendEvent:event];
		}
		return true;
	}

	Module* OSXHost::CreateModule(std::string& path)
	{
		void* lib_handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!lib_handle)
		{
			std::cerr << "Error load module: " << path << std::endl;
			return 0;
		}

		// get the module factory
		ModuleCreator* create = (ModuleCreator*)dlsym(lib_handle, "CreateModule");
		if (!create)
		{
			std::cerr << "Error load create entry from module: " << path << std::endl;
			return 0;
		}

		return create(this,FileUtils::GetDirectory(path));
	}
}

@interface KrollMainThreadCaller : NSObject
{
	kroll::BoundMethod *method;
	SharedPtr<kroll::Value> *result;
	ValueList* args;
	SharedPtr<kroll::Value> *exception;
}
- (id)initWithBoundMethod:(SharedPtr<kroll::BoundMethod>)method args:(ValueList*)args;
- (void)call;
- (SharedPtr<kroll::Value>)getResult;
- (SharedPtr<kroll::Value>)getException;
@end

@implementation KrollMainThreadCaller
- (id)initWithBoundMethod:(SharedPtr<kroll::BoundMethod>)m args:(ValueList*)a
{
	self = [super init];
	if (self)
	{
		method = m.get();
		args = a;
		result = new SharedPtr<kroll::Value>();
		exception = new SharedPtr<kroll::Value>();
	}
	return self;
}
- (void)dealloc
{
//	delete result;
//	delete args;
//	delete exception;
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
		result->assign(method->Call(*a));
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
}
@end

namespace kroll
{
	SharedValue OSXHost::InvokeMethodOnMainThread(SharedBoundMethod method,
	                                              const ValueList& args)
	{
		KrollMainThreadCaller *caller = [[[KrollMainThreadCaller alloc] initWithBoundMethod:method args:&args] autorelease];
		[caller performSelectorOnMainThread:@selector(call) withObject:nil waitUntilDone:YES];
		SharedValue exception = [caller getException];
		if (exception.isNull())
		{
			SharedValue result = [caller getResult];
			return result;
		}
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
