/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <signal.h>
#include "pythonmodule.h"
#include "pythontypes.h"
#include "pythontest.h"
	
namespace kroll
{
	KROLL_MODULE(PythonModule)

	PythonModule* PythonModule::instance_ = NULL;

	void PythonModule::Initialize()
	{
		KR_DUMP_LOCATION
		
		PythonModule::instance_ = this;

		//Py_SetProgramName(); //TODO: maybe we need to setup path to script?
		Py_Initialize();

		InitializeDefaultBindings(host);

		host->AddModuleProvider(this);
	}

	void PythonModule::Destroy()
	{
		KR_DUMP_LOCATION
		
		// clean up
		Py_Finalize();
		
		// FIXME - unregister / unbind?
		PythonModule::instance_ = NULL;
	}


	const static std::string python_suffix = "module.py";

	bool PythonModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-python_suffix.length()) == python_suffix);
	}

	Module* PythonModule::CreateModule(std::string& path)
	{
		char* path2 = (char*)malloc(sizeof(char)*path.length()+1);
		size_t length = path.copy(path2, path.length(), 0);
		path2[length] = '\0';

		std::cout << "Create module: " << path2 << std::endl;

		FILE *file = fopen(path2, "r");
		printf("got python file: %d\n", (int) file);
		
		//FIXME - we need to create a separate version of scope stuff

	#if !defined(OS_WIN32)
		// right now python is crashing in win32, need to investigate
		PyRun_SimpleFile(file,path2);
	#endif
		std::cout << "PyRan simple file" << std::endl;

		std::string path3(path2);
		free(path2);
		
		std::cout << "return new PythonModuleInstance " << path3 << std::endl;
		return new PythonModuleInstance(host, path3);
	}

	void PythonModule::Test()
	{
		PythonUnitTestSuite suite;
		suite.Run(host);
	}
}
