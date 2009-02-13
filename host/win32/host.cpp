/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "host.h"
#include <iostream>
#include <cstring>
#include <windows.h>
#include <commctrl.h>
#include <ole2.h>

namespace kroll
{
	bool Win32Host::ole_initialized = false;
	std::vector<std::pair<SharedBoundMethod, ValueList> > Win32Host::methodsToInvoke;

	/*static*/
	void Win32Host::InitOLE() {
		if (!ole_initialized) {
			OleInitialize(NULL);
			ole_initialized = true;
		}
	}

	Win32Host::Win32Host(HINSTANCE hInstance, int _argc, const char** _argv) : Host(_argc,_argv), instance_handle(hInstance)
	{
	}

	Win32Host::~Win32Host()
	{
		if (ole_initialized) {
			OleUninitialize();
		}
	}

	const char* Win32Host::GetPlatform()
	{
		return "win32";
	}

	const char* Win32Host::GetModuleSuffix()
	{
		return "module.dll";
	}

	bool Win32Host::Start()
	{
		Host::Start();
		thread_id = GetCurrentThreadId();
		return true;
	}

	bool Win32Host::RunLoop()
	{
		// just process one message at a time
		MSG message;
		if (GetMessage(&message, NULL, 0, 0))
		{
			if(message.message == WM_USER)
			{
				this->InvokeMethods();
			}
			else
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		return true;
	}

	Module* Win32Host::CreateModule(std::string& path)
	{
		HMODULE module = LoadLibraryA(path.c_str());

		if (!module)
		{
			fprintf(stderr, "Error loading module (%d): %s\n", GetLastError(), path.c_str());
			return 0;
		}

		// get the module factory
		ModuleCreator* create = (ModuleCreator*)GetProcAddress(module, "CreateModule");
		if (!create)
		{
			fprintf(stderr, "Couldn't find ModuleCreator entry point for %s\n", path.c_str());
			return 0;
		}

		return create(this,FileUtils::GetDirectory(path));
	}

	SharedValue Win32Host::InvokeMethodOnMainThread(SharedBoundMethod method,
                                                    const ValueList& args)
	{
		std::pair<SharedBoundMethod, ValueList> p(method, args);
		methodsToInvoke.push_back(p);

		// TODO use a different msg for this ..
		PostThreadMessage(thread_id, WM_USER, 0, 0);

		// TODO if we need to return the callback result, then need to add logic for that here
		// otherwise, change the method signature to not return a value
		return NULL;
	}

	void Win32Host::InvokeMethods()
	{
		while(this->methodsToInvoke.size() > 0)
		{
			std::pair<SharedBoundMethod, ValueList> p = this->methodsToInvoke.at(0);
			this->methodsToInvoke.erase(this->methodsToInvoke.begin());

			try
			{
				SharedValue result = p.first->Call(p.second);
			}
			catch (...)
			{
				std::cerr << "Exception occurred during JS callback" << std::endl;
			}
		}
	}
}

extern "C"
{
	int Execute(HINSTANCE hInstance, int argc, const char **argv){
		Host *host = new kroll::Win32Host(hInstance,argc,argv);
		return host->Run();
	}
}
