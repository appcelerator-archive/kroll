/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WIN32_HOST_H
#define _WIN32_HOST_H

#include <api/base.h>
#include <api/kroll.h>
#include <string>
#include <vector>
#include <windows.h>

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
		static void InitOLE();

		Win32Host(HINSTANCE hInstance, int argc, const char **argv);
		virtual ~Win32Host();

		virtual Module* CreateModule(std::string& path);

		HINSTANCE GetInstanceHandle() { return instance_handle; }
		SharedValue InvokeMethodOnMainThread(SharedBoundMethod method,
		                                     const ValueList& args);
		const char* GetPlatform();
		const char* GetModuleSuffix();

	protected:
		bool RunLoop();
		bool Start();

	private:
		HINSTANCE instance_handle;
		static bool ole_initialized;
		static std::vector<std::pair<SharedBoundMethod, ValueList> > methodsToInvoke;
		void InvokeMethods();
		DWORD thread_id;
	};
}

extern "C"
{
	KROLL_HOST_API int Execute(HINSTANCE hInstance, int argc, const char **argv);
}

#endif
