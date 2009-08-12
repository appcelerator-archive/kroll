/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "event_window.h"
namespace kroll
{
	const wchar_t* EventWindow::windowClassName = L"Kroll Event Window";
	const WNDCLASSW* EventWindow::windowClass = NULL;

	EventWindow::EventWindow(HINSTANCE hInstance) :
		hInstance(hInstance),
		handle(NULL),
		logger(Logger::Get("EventWindow"))
	{
		InitializeWindowClass(hInstance);

		this->handle = ::CreateWindowW(
			EventWindow::windowClassName,
			L"Kroll Event Window Instance",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			this->hInstance,
			NULL);

		if (!this->handle)
		{
			std::string error("Could not create event window: ");
			error.append(Win32Utils::QuickFormatMessage(GetLastError()));
			logger->Critical(error);
		}

		SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	EventWindow::~EventWindow()
	{
		this->DestroyWindow();
	}

	void EventWindow::InitializeWindowClass(HINSTANCE hInstance)
	{
		if (!windowClass)
		{
			static WNDCLASSW windowClassPrivate;
			ZeroMemory(&windowClassPrivate, sizeof(WNDCLASS)); 
			windowClassPrivate.lpfnWndProc = EventWindow::WindowProcedure;
			windowClassPrivate.hInstance = hInstance;
			windowClassPrivate.lpszClassName = EventWindow::windowClassName;
			windowClass = &windowClassPrivate;
			ATOM result = ::RegisterClassW(windowClass);

			if (!result)
			{
				std::string error("Could not register event window class: ");
				error.append(Win32Utils::QuickFormatMessage(GetLastError()));
				Logger::Get("EventWindow")->Critical(error);
			}
		}
	}

	void EventWindow::DestroyWindow()
	{
		if (this->handle)
		{
			BOOL result = ::DestroyWindow(this->handle);

			if (result == 0)
			{
				// Cannot uset the Logger here, because this happens after
				// logging has completely shut down.
				std::string error("Could not destroy event window: ");
				error.append(Win32Utils::QuickFormatMessage(GetLastError()));
				fprintf(stderr, error.c_str());
			}
			this->handle = NULL;
		}
	}

	HWND EventWindow::AddMessageHandler(MessageHandler handler)
	{
		if (this->handle)
		{
			Poco::Mutex::ScopedLock lock(handlersMutex);
			for (int i = 0; i < handlers.size(); i++)
			{
				MessageHandler h = handlers[i];
				if (h == handler)
					return this->handle;
			}

			handlers.push_back(handler);
		}

		return this->handle;
	}

	LRESULT CALLBACK EventWindow::Handler(
		HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
	{
		{
			Poco::Mutex::ScopedLock lock(handlersMutex);
			for (int i = 0; i < handlers.size(); i++)
			{
				MessageHandler h = handlers[i];
				if (h(hwnd, message, wParam, lParam))
					return 0;
			}
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	/*static*/
	LRESULT CALLBACK EventWindow::WindowProcedure(
		HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
	{
		EventWindow* window =
			reinterpret_cast<EventWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		if (window)
			return window->Handler(hwnd, message, wParam, lParam);
		else
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
}

