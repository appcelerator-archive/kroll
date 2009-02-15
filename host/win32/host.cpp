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
#include "win32_job.h"
 
using Poco::ScopedLock;
using Poco::Mutex;

#define WM_JOB_TICKLE_REQUEST WM_USER

namespace kroll
{
	bool Win32Host::ole_initialized = false;
	
	/*static*/
	void Win32Host::InitOLE() {
		if (!ole_initialized) {
			OleInitialize(NULL);
			ole_initialized = true;
		}
	}

	Win32Host::Win32Host(HINSTANCE hInstance, int _argc, const char** _argv) : Host(_argc,_argv), instance_handle(hInstance)
	{
		InitOLE();
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
	
	Poco::Mutex& Win32Host::GetJobQueueMutex()
	{
		return this->job_queue_mutex;
	}
 
	std::vector<Win32Job*>& Win32Host::GetJobs()
	{
		return this->jobs;
	}

	bool Win32Host::RunLoop()
	{
		// just process one message at a time
		MSG message;
		if (GetMessage(&message, NULL, 0, 0))
		{
			if(message.message == WM_JOB_TICKLE_REQUEST)
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
		std::cout << "+++ INVOKE ON MAIN THREAD " << std::endl;
		Win32Job* job = new Win32Job(method, args);
		{
			Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
			this->jobs.push_back(job); // Enqueue job
		}
		std::cout << "+++ INVOKE 1" << std::endl;
		// send a message to tickle the windows message queue
		PostThreadMessage(thread_id, WM_JOB_TICKLE_REQUEST, 0, 0);
		std::cout << "+++ INVOKE 2" << std::endl;
		job->Wait(); // Wait for processing
		std::cout << "+++ INVOKE 3" << std::endl;
 
		SharedValue r = job->GetResult();
		std::cout << "+++ INVOKE 4" << std::endl;
		ValueException e = job->GetException();
		std::cout << "+++ INVOKE 5" << std::endl;
		delete job;
		std::cout << "+++ INVOKE 6" << std::endl;
 
		if (!r.isNull())
			return r;
		else
			throw e;
	}

	void Win32Host::InvokeMethods()
	{
std::cout << "+++ ENTER INVOKE METHODS " << std::endl;
	
		// Prevent other threads trying to queue jobs.
		Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
std::cout << "+++ ENTER INVOKE METHODS 1" << std::endl;
		std::vector<Win32Job*>& jobs = this->GetJobs();
std::cout << "+++ ENTER INVOKE METHODS 2 = " << jobs.size() << std::endl;
 
		if (jobs.size() == 0)
			return;
		
std::cout << "+++ ENTER INVOKE METHODS 3" << std::endl;
		std::vector<Win32Job*>::iterator j;
		for (j = jobs.begin(); j != jobs.end(); j++)
		{
std::cout << "+++ BEFORE EXECUTE" << std::endl;
			(*j)->Execute();
std::cout << "+++ AFTER EXECUTE" << std::endl;
		}
 
		jobs.clear();
std::cout << "+++ EXIT INVOKE METHODS " << std::endl;
	}
}

extern "C"
{
	int Execute(HINSTANCE hInstance, int argc, const char **argv){
		Host *host = new kroll::Win32Host(hInstance,argc,argv);
		return host->Run();
	}
}
