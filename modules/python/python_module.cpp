/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "python_module.h"

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

		this->InitializeBinding();

		host->AddModuleProvider(this);
	}

	void PythonModule::Stop()
	{
		KR_DUMP_LOCATION

		// clean up
		Py_Finalize();

		// FIXME - unregister / unbind?
		PythonModule::instance_ = NULL;
	}

	void PythonModule::InitializeBinding()
	{
		SharedBoundObject global = this->host->GetGlobalObject();

		SharedBoundObject binding = new StaticBoundObject();
		global->Set("Python", Value::NewObject(binding));

		SharedBoundMethod evaluator = new PythonEvaluator();
		binding->Set("evaluate", Value::NewMethod(evaluator));

		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* main_dict = PyModule_GetDict(main_module);
		PyObject* api = PythonUtils::KObjectToPyObject(global);
		PyDict_SetItemString(main_dict, PRODUCT_NAME, api);
		Py_DECREF(api);
	}


	const static std::string python_suffix = "module.py";

	bool PythonModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-python_suffix.length()) == python_suffix);
	}

	Module* PythonModule::CreateModule(std::string& path)
	{
		std::cout << "Create module: " << path << std::endl;
		FILE *file = fopen(path.c_str(), "r");

		PyRun_SimpleFile(file,path.c_str());
		std::cout << "PyRan simple file" << std::endl;

		std::cout << "return new PythonModuleInstance " << path << std::endl;
		return new PythonModuleInstance(host, path);
	}

	void PythonModule::Test()
	{
		PythonUnitTestSuite suite;
		suite.Run(host);
	}
}
