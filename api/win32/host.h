/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WIN32_HOST_H
#define _WIN32_HOST_H

#include <api/kroll.h>
#include <string>
#include <vector>
#include <windows.h>
#include <Poco/ScopedLock.h>
#include <Poco/Mutex.h>
#include <Poco/Condition.h>
#include "event_window.h"
 
namespace kroll
{
	class KROLL_API Win32Host : public Host
	{
	public:
		Win32Host(HINSTANCE hInstance, int argc, const char **argv);
		virtual ~Win32Host();
		static void InitOLE();
		virtual Module* CreateModule(std::string& path);
		const char* GetPlatform();
		const char* GetModuleSuffix();
		virtual bool IsMainThread();

		HWND AddMessageHandler(MessageHandler handler);
		HINSTANCE GetInstanceHandle() { return instanceHandle; }
		HWND GetEventWindow() { return eventWindow.GetHandle(); }
		static inline Win32Host* Win32Instance()
		{
			return static_cast<Win32Host*>(Host::GetInstance());
		}

	protected:
		virtual void SignalNewMainThreadJob();
		virtual bool RunLoop();
		virtual bool Start();

	private:
		HINSTANCE instanceHandle;
		static bool oleInitialized;
		DWORD mainThreadId;
		EventWindow eventWindow;
	};
}

extern "C"
{
	KROLL_API int Execute(HINSTANCE hInstance, int argc, const char **argv);
}

#endif
