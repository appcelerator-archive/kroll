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

#define MAX_CONSOLE_LINES 500;

using Poco::ScopedLock;
using Poco::Mutex;

namespace kroll
{
	bool Win32Host::oleInitialized = false;
	static UINT tickleRequestMessage =
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

	Win32Host::Win32Host(HINSTANCE hInstance, int argc, const char** argv) :
		Host(argc, argv),
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
		// Windows DLLs often load libraries dynamically via LoadLibrary and expect
		// dependencies to be on the DLL search path. Thus we really can't restore
		// the original path here if we want things to continue working properly.
		// This shouldn't be too much of an issue either, as long as programs that
		// we launch rely on the safe dll search path.
		// string origPath(EnvironmentUtils::Get("KR_ORIG_PATH"));
		// EnvironmentUtils::Set("PATH", origPath);

		Host::Start();
		mainThreadId = GetCurrentThreadId();
		return true;
	}

	bool Win32Host::RunLoop()
	{
		static bool postedQuitMessage = false;

		// Just process one message at a time
		MSG message;
		if (GetMessage(&message, NULL, 0, 0))
		{
			if (message.message == tickleRequestMessage)
				this->RunMainThreadJobs();

			// Always translate/dispatch this message, in case
			// we are polluting the message namespace
			// .. i'm looking at you flash!
			TranslateMessage(&message);
			DispatchMessage(&message);

			// Yo, just got word that it's time to exit. Post a quit
			// message that this  loop will soon see as a WM_QUIT.
			// Only quit after processing that message.
			if (this->exiting && !postedQuitMessage)
			{
				PostQuitMessage(this->exitCode);
				postedQuitMessage = true;
			}

			return true;
		}
		else
		{
			return false; // Got the WM_QUIT message.
		}
	}

	Module* Win32Host::CreateModule(std::string& path)
	{
		std::wstring widePath(UTF8ToWide(path));
		HMODULE module = LoadLibraryExW(widePath.c_str(),
			NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

		if (!module)
		{
			throw ValueException::FromFormat("Error loading module (%d): %s: %s\n",
				GetLastError(), path.c_str(),
				Win32Utils::QuickFormatMessage(GetLastError()).c_str());
		}

		// get the module factory
		ModuleCreator* create = (ModuleCreator*)GetProcAddress(module, "CreateModule");
		if (!create)
		{
			throw ValueException::FromFormat(
				"Couldn't find ModuleCreator entry point for %s\n", path.c_str());
		}

		return create(this, FileUtils::GetDirectory(path).c_str());
	}

	bool Win32Host::IsMainThread()
	{
		return mainThreadId == GetCurrentThreadId();
	}

	void Win32Host::SignalNewMainThreadJob()
	{
		// send a message to tickle the windows message queue
		PostThreadMessage(mainThreadId, tickleRequestMessage, 0, 0);
	}

	HWND Win32Host::AddMessageHandler(MessageHandler handler)
	{
		return eventWindow.AddMessageHandler(handler);
	}
}

extern "C"
{
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
		fp = _fdopen(hConHandle, "w");

		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);

		// redirect unbuffered STDIN to the console
		lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

		fp = _fdopen(hConHandle, "r");
		*stdin = *fp;
		setvbuf(stdin, NULL, _IONBF, 0);

		// redirect unbuffered STDERR to the console
		lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stderr = *fp;
		setvbuf(stderr, NULL, _IONBF, 0);

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
		// point to console as well
		std::ios::sync_with_stdio();
	}

	int Execute(HINSTANCE hInstance, int argc, const char **argv)
	{
		Host *host = new kroll::Win32Host(hInstance,argc,argv);
#ifndef DEBUG
		// only create a debug console when not compiled in debug mode 
		// otherwise, it should be autocreated
		if (host->DebugModeEnabled())
		{
			RedirectIOToConsole();
		}
#endif
		return host->Run();
	}
}
