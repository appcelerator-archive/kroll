/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

namespace kroll
{

static const wchar_t* windowClassName = L"Kroll Event Window";

LRESULT CALLBACK WindowProcedure(HWND hwnd, unsigned int message, WPARAM wParam,
	 LPARAM lParam)
{
	EventWindow* window =
		reinterpret_cast<EventWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (window)
		return window->Handler(hwnd, message, wParam, lParam);
	else
		return DefWindowProc(hwnd, message, wParam, lParam);
}

static void InitializeWindowClass(HINSTANCE hInstance)
{
	static bool initialized = false;
	if (initialized)
		return;

	WNDCLASSW windowClassPrivate;
	ZeroMemory(&windowClassPrivate, sizeof(WNDCLASS)); 
	windowClassPrivate.lpfnWndProc = WindowProcedure;
	windowClassPrivate.hInstance = hInstance;
	windowClassPrivate.lpszClassName = windowClassName;
	ATOM result = ::RegisterClassW(&windowClassPrivate);

	if (!result)
	{
		std::string error("Could not register event window class: ");
		error.append(Win32Utils::QuickFormatMessage(GetLastError()));
		Logger::Get("EventWindow")->Critical(error);
		return;
	}

	initialized = true;
}

EventWindow::EventWindow() :
	handle(NULL),
	logger(Logger::Get("EventWindow"))
{
	HINSTANCE instanceHandle = GetModuleHandle(NULL);
	InitializeWindowClass(instanceHandle);

	this->handle = ::CreateWindowW(
		windowClassName,
		L"Kroll Event Window Instance",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instanceHandle,
		0);

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
}

