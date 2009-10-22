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
#include "win32_job.h"
#include "event_window.h"
 
#ifdef KROLL_HOST_EXPORT
# define KROLL_HOST_API __declspec(dllexport)
#else
# define KROLL_HOST_API __declspec(dllimport)
#endif

namespace kroll
{
	class KROLL_HOST_API Win32Host : public Host
	{
	public:
		Win32Host(HINSTANCE hInstance, int argc, const char **argv);
		virtual ~Win32Host();

		static inline SharedPtr<Win32Host> Win32Instance()
		{
			return Host::GetInstance().cast<Win32Host>();
		}

		static void InitOLE();
		virtual Module* CreateModule(std::string& path);

		HINSTANCE GetInstanceHandle() { return instanceHandle; }
		KValueRef InvokeMethodOnMainThread(KMethodRef method, 
			const ValueList& args, bool waitForCompletion=true);
		const char* GetPlatform();
		const char* GetModuleSuffix();
		HWND AddMessageHandler(MessageHandler handler);
		HWND GetEventWindow() { return eventWindow.GetHandle(); }
		
	protected:
		bool RunLoop();
		bool Start();
		Poco::Mutex& GetJobQueueMutex();
		std::vector<Win32Job*>& GetJobs();
		static UINT tickleRequestMessage;
		EventWindow eventWindow;

	private:
		HINSTANCE instanceHandle;
		static bool oleInitialized;
		void InvokeMethods();
		DWORD threadId;
		Poco::Mutex jobQueueMutex;
		std::vector<Win32Job*> jobs;
	};
}

extern "C"
{
	KROLL_HOST_API int Execute(HINSTANCE hInstance, int argc, const char **argv);
}

#endif
