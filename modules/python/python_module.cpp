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

		//TODO: maybe we need to setup path to script?
		//Py_SetProgramName(); 
		PyObject *path = PySys_GetObject((char*) "path");
		PyObject *s = PyString_FromString(host->GetApplication()->GetResourcesPath().c_str());
		PyList_Insert(path, 0, s);
		Py_XDECREF(s);

		this->InitializeBinding();
		this->InitializeModule();
		host->AddModuleProvider(this);
	}

	void PythonModule::Stop()
	{
		SharedKObject global = this->host->GetGlobalObject();
		global->Set("Python", Value::Undefined);
		this->binding->Set("evaluate", Value::Undefined);
		this->binding = NULL;
		PythonModule::instance_ = NULL;

		Py_Finalize();
	}

	void PythonModule::InitializeBinding()
	{
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new StaticBoundObject();
		global->Set("Python", Value::NewObject(this->binding));

		SharedKMethod evaluator = new PythonEvaluator();
		/**
		 * @tiapi(method=True,name=Python.evaluate,since=0.2) Evaluates a string as python code
		 * @tiarg(for=Python.evaluate,name=code,type=String) python script code
		 * @tiarg(for=Python.evaluate,name=scope,type=Object) global variable scope
		 * @tiresult(for=Python.evaluate,type=any) the result of the evaluation
		 */
		this->binding->Set("evaluate", Value::NewMethod(evaluator));

		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* main_dict = PyModule_GetDict(main_module);
		PyObject* api = PythonUtils::KObjectToPyObject(Value::NewObject(global));
		PyDict_SetItemString(main_dict, PRODUCT_NAME, api);
		Py_DECREF(api);
	}

	void PythonModule::InitializeModule()
	{
		/*
		We need to build the titanium module
		to be imported by python modules.
		*/
		PyObject* titanium_module = PyImport_AddModule("titanium");
 
		// Add global object
		SharedKObject global = this->host->GetGlobalObject();
		PyObject* api = PythonUtils::KObjectToPyObject(Value::NewObject(global));
		PyModule_AddObject(titanium_module, "Titanium", api);

		// Add Module class
		PyObject* classDict = PyDict_New();
		PyObject* className = PyString_FromString("Module");
		PyObject* moduleClass = PyClass_New(NULL, classDict, className);
		PyModule_AddObject(titanium_module, "Module", moduleClass);
		Py_DECREF(classDict);
		Py_DECREF(className);
	}

	const static std::string python_suffix = "module.py";

	bool PythonModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-python_suffix.length()) == python_suffix);
	}

	Module* PythonModule::CreateModule(std::string& path)
	{
		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-python_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		Logger *logger = Logger::Get("Python");
		logger->Info("Loading Python path=%s", path.c_str());

		PythonModuleInstance* instance = new PythonModuleInstance(host, path, moduledir, name);
		instance->Initialize();
		return instance;
	}

}
