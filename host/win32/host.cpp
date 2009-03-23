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
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "win32_job.h"

using Poco::ScopedLock;
using Poco::Mutex;

#define WM_JOB_TICKLE_REQUEST WM_USER+1

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

		std::string dir = FileUtils::GetDirectory(path);
		return create(this, dir.c_str());
	}

	SharedValue Win32Host::InvokeMethodOnMainThread(
		SharedKMethod method,
		const ValueList& args,
		bool synchronous)
	{
		Win32Job* job = new Win32Job(method, args, synchronous);
		if (thread_id == GetCurrentThreadId()) {
			job->Execute();
		}
		else
		{
			Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
			this->jobs.push_back(job); // Enqueue job
		}

		// send a message to tickle the windows message queue
		PostThreadMessage(thread_id, WM_JOB_TICKLE_REQUEST, 0, 0);

		if (!synchronous)
		{
			return Value::Undefined; // Handler will cleanup
		}
		else
		{
			// If this is the main thread, Wait() will fall
			// through because we've already called Execute() above.
			job->Wait(); // Wait for processing

			SharedValue r = job->GetResult();
			ValueException e = job->GetException();
			delete job;

			if (!r.isNull())
				return r;
			else
				throw e;
		}
	}

	void Win32Host::InvokeMethods()
	{
		// Prevent other threads trying to queue jobs.
		Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());

		std::vector<Win32Job*>& jobs = this->GetJobs();
		if (jobs.size() == 0)
			return;

		std::vector<Win32Job*>::iterator j = jobs.begin();
		while (j != jobs.end())
		{
			Win32Job* job = *j;
			j = jobs.erase(j);

			// Job might be freed soon after Execute()
			bool asynchronous = !job->IsSynchronous();
			job->Execute();

			if (asynchronous)
			{
				job->PrintException();
				delete job;
			}
		}

	}
}

extern "C"
{
	static const WORD MAX_CONSOLE_LINES = 500;

	void RedirectIOToConsole() {
		int hConHandle;
		long lStdHandle;
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		FILE *fp;

		// allocate a console for this app
		AllocConsole();

		// set the screen buffer to be big enough to let us scroll text
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
		coninfo.dwSize.Y = MAX_CONSOLE_LINES;

		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

		// redirect unbuffered STDOUT to the console
		lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen( hConHandle, "w" );

		*stdout = *fp;
		setvbuf( stdout, NULL, _IONBF, 0 );

		// redirect unbuffered STDIN to the console
		lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

		fp = _fdopen( hConHandle, "r" );
		*stdin = *fp;
		setvbuf( stdin, NULL, _IONBF, 0 );

		// redirect unbuffered STDERR to the console
		lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen( hConHandle, "w" );
		*stderr = *fp;
		setvbuf( stderr, NULL, _IONBF, 0 );

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
		// point to console as well
		std::ios::sync_with_stdio();
	}

	int Execute(HINSTANCE hInstance, int argc, const char **argv){
		Host *host = new kroll::Win32Host(hInstance,argc,argv);
#ifndef DEBUG
		// only create a debug console when not compiled in debug mode -- otherwise, it should be autocreated

		if (host->IsDebugMode()) {
			RedirectIOToConsole();
		}
#endif
		return host->Run();
	}
}
