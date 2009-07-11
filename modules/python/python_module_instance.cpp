/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module_instance.h"

namespace kroll
{
	PythonModuleInstance::PythonModuleInstance(Host *host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"), path(path)
	{
    try
		{
      this->Load();    // load source code from file
			this->Compile(); // compile code into module
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("Python");
			logger->Error("Could not load %s because %s", path.c_str(), (*ss).c_str());
		}
	}

	PythonModuleInstance::~PythonModuleInstance()
	{
	}

  void PythonModuleInstance::Load()
  {
    this->code = "";
		std::ifstream py_file(this->path.c_str());
		if (!py_file.is_open())
		{
			throw ValueException::FromString("Could not read Python file");
		}

		std::string line;
		while (!py_file.eof() )
		{
			std::getline(py_file, line);
			this->code.append(line);
			this->code.append("\n");
		}
		py_file.close();
  }

  void PythonModuleInstance::Compile()
  {
    const char* name = this->GetName().c_str();

    // compile code string
    PyObject* co = Py_CompileString(this->code.c_str(), name, Py_file_input);
    if (co == NULL)
    {
      PyErr_Print();
      throw ValueException::FromString("Cound not compile Python file");
    }

    // load module
    this->module = PyImport_ExecCodeModule((char*)name, co);
    Py_DECREF(co);
    if (this->module == NULL)
    {
      PyErr_Print();
      throw ValueException::FromString("Cound not load Python module");
    }

    this->code = "";
  }

	void PythonModuleInstance::Initialize () 
	{
	}

	void PythonModuleInstance::Destroy () 
	{
    // release module
    Py_XDECREF(this->module);
	}
}

