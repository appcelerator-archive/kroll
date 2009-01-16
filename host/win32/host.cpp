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
		std::string p(getenv("KR_PLUGINS"));
		std::string delimiter(";");
		FileUtils::Tokenize(p,this->module_paths,delimiter);

	}

	Win32Host::~Win32Host()
	{
		if (ole_initialized) {
			OleUninitialize();
		}
	}

	int Win32Host::Run()
	{
		std::vector<std::string>::iterator iter = this->module_paths.begin();
		while (iter!=this->module_paths.end())
		{
			std::string path = (*iter++);
			printf("Finding modules..\n");
			this->FindModules(path,this->modules);
		}

		// load our modules through the host implementation but let
		// the base class do the hard work for us
		this->LoadModules(this->modules);

		printf("Kroll Started (Win32)\n");

		MSG message;
		while (GetMessage(&message, NULL, 0, 0))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		return 0;
	}

	Module* Win32Host::CreateModule(std::string& path)
	{
		std::cout << "Win32Host::CreateModule " << path.c_str() << std::endl;

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

		std::cout << "Win32Host creating module ..." << std::endl;

		return create(this,FileUtils::GetDirectory(path));
	}
}


