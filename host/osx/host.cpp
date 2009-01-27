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
#include "log.h"

namespace kroll
{
	OSXHost::OSXHost(int _argc, const char **_argv) : Host(_argc,_argv)
	{
		SetupLog(_argc,_argv,[NSString stringWithFormat:@"%s/run.log",this->GetApplicationHome().c_str()]);

		char *p = getenv("KR_PLUGINS");
		if (p)
		{
			FileUtils::Tokenize(p, this->module_paths, ":");
		}
	}

	OSXHost::~OSXHost()
	{
		CloseLog();
	}

	int OSXHost::Run()
	{
		TRACE(@PRODUCT_NAME" Kroll Running (OSX)...");
		this->AddModuleProvider(this);
		this->LoadModules();

		[NSApp run];
		return 0;
	}

	Module* OSXHost::CreateModule(std::string& path)
	{
		std::cout << "Creating module " << path << std::endl;


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
	SharedPtr<kroll::BoundMethod> *method;
	SharedPtr<kroll::Value> *result;
	SharedPtr<kroll::ValueList> *args;
}
- (id)initWithBoundMethod:(SharedPtr<kroll::BoundMethod>)method args:(SharedPtr<ValueList>)args;
- (void)call;
- (SharedPtr<kroll::Value>)getResult;
@end

@implementation KrollMainThreadCaller
- (id)initWithBoundMethod:(SharedPtr<kroll::BoundMethod>)m args:(SharedPtr<ValueList>)a
{
	self = [super init];
	if (self)
	{
		method = new SharedPtr<kroll::BoundMethod>(m);
		args = new SharedPtr<kroll::ValueList>(a);
		result = new SharedPtr<kroll::Value>();
	}
	return self;
}
- (void)dealloc
{
	delete method;
	delete result;
	delete args;
	[super dealloc];
}
- (SharedPtr<kroll::Value>)getResult
{
	return *result;
}
- (void)call
{
	kroll::ValueList a;
	if (!args->isNull())
	{
		ValueList::iterator i = (*args)->begin();
		while (i!=(*args)->end())
		{
			a.push_back((*i++));
		}
	}
	result->assign((*method)->Call(a));
}
@end

namespace ti
{
	SharedValue OSXHost::InvokeMethodOnMainThread(SharedBoundMethod method,
	                                              SharedPtr<ValueList> args)
	{
		KrollMainThreadCaller *caller = [[KrollMainThreadCaller alloc] initWithBoundMethod:method args:args];
		[caller performSelectorOnMainThread:@selector(call) withObject:nil waitUntilDone:YES];
		SharedValue result = [caller getResult];
		[caller release];
		return result;
	}
}

