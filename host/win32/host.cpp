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

namespace kroll
{
	bool Win32Host::oleInitialized = false;
	UINT Win32Host::tickleRequestMessage =
		::RegisterWindowMessageA(PRODUCT_NAME"TickleRequest");

	/*static*/
	void Win32Host::InitOLE()
	{
		if (!oleInitialized)
		{
			OleInitialize(NULL);
			oleInitialized = true;
		}
	}

	Win32Host::Win32Host(HINSTANCE hInstance, int _argc, const char** _argv) :
		Host(_argc,_argv),
		instanceHandle(hInstance),
		eventWindow(hInstance)
	{
		InitOLE();
	}

	Win32Host::~Win32Host()
	{
		if (oleInitialized)
		{
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
		threadId = GetCurrentThreadId();
		return true;
	}

	Poco::Mutex& Win32Host::GetJobQueueMutex()
	{
		return this->jobQueueMutex;
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
			if (message.message == tickleRequestMessage)
			{
				this->InvokeMethods();
			}
			
			// still translate/dispatch this message, in case
			// we are polluting the message namespace
			// .. i'm looking at you flash!
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		return true;
	}

	Module* Win32Host::CreateModule(std::string& path)
	{
		std::wstring widePath = UTF8ToWide(path);
		HMODULE module = LoadLibraryW(widePath.c_str());

		if (!module)
		{
			throw ValueException::FromFormat("Error loading module (%d): %s: %s\n",
				GetLastError(), path.c_str(), kroll::Win32Utils::QuickFormatMessage(GetLastError()).c_str());
		}

		// get the module factory
		ModuleCreator* create = (ModuleCreator*)GetProcAddress(module, "CreateModule");
		if (!create)
		{
			throw ValueException::FromFormat("Couldn't find ModuleCreator entry point for %s\n", path.c_str());
		}

		std::string dir = FileUtils::GetDirectory(path);
		return create(this, dir.c_str());
	}

	KValueRef Win32Host::InvokeMethodOnMainThread(
		KMethodRef method,
		const ValueList& args,
		bool synchronous)
	{
		Win32Job* job = new Win32Job(method, args, synchronous);
		if (threadId == GetCurrentThreadId() && synchronous)
		{
			job->Execute();
		}
		else
		{
			Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
			this->jobs.push_back(job); // Enqueue job
		}

		// send a message to tickle the windows message queue
		PostThreadMessage(threadId, tickleRequestMessage, 0, 0);

		if (!synchronous)
		{
			return Value::Undefined; // Handler will cleanup
		}
		else
		{
			// If this is the main thread, Wait() will fall
			// through because we've already called Execute() above.
			job->Wait(); // Wait for processing

			KValueRef r = job->GetResult();
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
		// Prevent other threads trying to queue while we clear the queue.
		// But don't block the invocation task while we actually execute
		// the jobs -- one of these jobs may try to add something to the
		// job queue -- deadlock-o-rama
		std::vector<Win32Job*> myJobs;
		{
			Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
			myJobs = this->jobs;	
			this->jobs.clear();
		}

		std::vector<Win32Job*>::iterator j = myJobs.begin();
		while (j != myJobs.end())
		{
			Win32Job* job = *j++;

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

	void RedirectIOToConsole()
	{
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

		if (host->IsDebugMode())
		{
			RedirectIOToConsole();
		}
#endif
		return host->Run();
	}

	HWND Win32Host::AddMessageHandler(MessageHandler handler)
	{
		return eventWindow.AddMessageHandler(handler);
	}
}
