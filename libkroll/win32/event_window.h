/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _HOST_EVENT_WINDOW_H
#define _HOST_EVENT_WINDOW_H

#include <vector>
#include <Poco/Mutex.h>

typedef bool (*MessageHandler)(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);

namespace kroll
{
	class Logger;
	class EventWindow
	{
	public:
		EventWindow();
		virtual ~EventWindow();
		void DestroyWindow();
		HWND AddMessageHandler(MessageHandler handler);
		LRESULT CALLBACK Handler(
			HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);
		HWND GetHandle() { return handle; }
		
	private:
		HWND handle;
		Logger* logger;
		std::vector<MessageHandler> handlers;
		Poco::Mutex handlersMutex;
	};
}

#endif
