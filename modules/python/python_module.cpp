/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "python_module.h"
#include <Poco/Path.h>

namespace kroll
{
	KROLL_MODULE(PythonModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	PythonModule* PythonModule::instance_ = NULL;

	void PythonModule::Initialize()
	{
		PythonModule::instance_ = this;

		Py_Initialize();
		PyEval_InitThreads();
		PyEval_SaveThread();

		{
		PyLockGIL lock;

		PyObject* path = PySys_GetObject((char*) "path");
		PyObject* s = PyString_FromString(
			host->GetApplication()->GetResourcesPath().c_str());
		PyList_Insert(path, 0, s);
		Py_XDECREF(s);
		}

		PythonUtils::InitializePythonKClasses();
		this->InitializeBinding();
		host->AddModuleProvider(this);
	}

	void PythonModule::Stop()
	{
		// Don't release this GIL state because by the time we're 
		// done here, the interpreter will have bitten the dust
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure();

		SharedKObject global = this->host->GetGlobalObject();
		global->Set("Python", Value::Undefined);
		Script::GetInstance()->RemoveScriptEvaluator(this->binding);
		this->binding = NULL;
		PythonModule::instance_ = NULL;
		Py_Finalize();
	}

	void PythonModule::InitializeBinding()
	{
		PyLockGIL lock;
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new PythonEvaluator();
		global->Set("Python", Value::NewObject(this->binding));
		Script::GetInstance()->AddScriptEvaluator(this->binding);
		
		{
			PyObject* main_module = PyImport_AddModule("__main__");
			PyObject* main_dict = PyModule_GetDict(main_module);
			PyObject* api = PythonUtils::KObjectToPyObject(Value::NewObject(global));
			PyDict_SetItemString(main_dict, PRODUCT_NAME, api);
			Py_DECREF(api);
		}
	}


	const static std::string python_suffix = "module.py";

	bool PythonModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-python_suffix.length()) == python_suffix);
	}

	Module* PythonModule::CreateModule(std::string& path)
	{
		PyLockGIL lock;
		FILE *file = fopen(path.c_str(), "r");

		PyRun_SimpleFile(file,path.c_str());

		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-python_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		Logger *logger = Logger::Get("Python");
		logger->Info("Loading Python path=%s", path.c_str());

		return new PythonModuleInstance(host, path, moduledir, name);
	}

}
