/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _HOST_EVENT_WINDOW_H
#define _HOST_EVENT_WINDOW_H
#include <kroll/kroll.h>
#include <vector>

typedef bool (*MessageHandler)(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);
namespace kroll
{
	class EventWindow
	{
		public:
		EventWindow(HINSTANCE hInstance);
		virtual ~EventWindow();
		void DestroyWindow();
		HWND AddMessageHandler(MessageHandler handler);
		LRESULT CALLBACK Handler(
			HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);
		HWND GetHandle() { return handle; }
		
		private:
		HINSTANCE hInstance;
		HWND handle;
		Logger* logger;
		std::vector<MessageHandler> handlers;
		Poco::Mutex handlersMutex;
		static const wchar_t* windowClassName;
		static const WNDCLASSW* windowClass;
		static void InitializeWindowClass(HINSTANCE hInstance);
		static LRESULT CALLBACK WindowProcedure(
			HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);
	};
}

#endif
