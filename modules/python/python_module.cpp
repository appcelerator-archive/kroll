/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module.h"

#include <signal.h>
#include <Poco/Path.h>

extern "C" EXPORT PythonModule* CreateModule(Host *host, const char* path)
{
	return new PythonModule(host, path);
}

static const char* supportedScriptTypes[3] = {"py", "python", 0};

namespace kroll
{
	PythonModule* PythonModule::instance_ = NULL;

	static void PyListInsertString(
		PyObject *list, int index, std::string string)
	{
		PyObject* s = PyString_FromString(string.c_str());
		PyList_Insert(list, 0, s);
		Py_XDECREF(s);
	}
	
	void PythonModule::Initialize()
	{
		PythonModule::instance_ = this;

		Py_Initialize();
		PyEval_InitThreads();
		PyEval_SaveThread();

		{
			PyLockGIL lock;

			PyObject* path = PySys_GetObject((char*) "path");
			PyListInsertString(path, 0, 
				UTF8ToSystem(host->GetApplication()->GetResourcesPath()).c_str());
#ifdef OS_WIN32
			PyListInsertString(path, 0, FileUtils::Join(
				UTF8ToSystem(this->GetPath()).c_str(), "DLLs", NULL));
			PyListInsertString(path, 0, FileUtils::Join(
				UTF8ToSystem(this->GetPath()).c_str(), "Lib", NULL));
			PyListInsertString(path, 0, FileUtils::Join(
				UTF8ToSystem(this->GetPath()).c_str(), "Lib", "lib-tk", NULL));
#endif
		}

        host->script()->AddInterpreter(&interpreter, supportedScriptTypes);

		PythonUtils::InitializePythonKClasses();
		host->AddModuleProvider(this);
	}

	void PythonModule::Stop()
	{
		// Don't release this GIL state because by the time we're 
		// done here, the interpreter will have bitten the dust
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure();

        host->script()->RemoveInterpreter(&interpreter);

		PythonModule::instance_ = NULL;
		Py_Finalize();
	}

	const static std::string python_suffix = "module.py";

	bool PythonModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-python_suffix.length()) == python_suffix);
	}

	Module* PythonModule::CreateModule(std::string& path)
	{
		PyLockGIL lock;
		path = UTF8ToSystem(path);
		FILE* file = fopen(path.c_str(), "r");
		PyRun_SimpleFileEx(file, path.c_str(), 1);

		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-python_suffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);
		return new PythonModuleInstance(host, path, moduledir, name);
	}
}
