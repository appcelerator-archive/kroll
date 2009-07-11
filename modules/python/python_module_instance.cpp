/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module_instance.h"

namespace kroll
{
	// TODO: Implement real method metadata and lifecycle events for
	// scripting language-based modules
	PythonModuleInstance::PythonModuleInstance(Host *host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"), path(path)
	{
    try
		{
      this->Load()
			this->Compile()
		}
		catch (ValueException& e)
		{
			SharedString ss = e.GetValue()->DisplayString();
			Logger *logger = Logger::Get("Python");
			logger->Error("Could not execute %s because %s", path.c_str(), (*ss).c_str());
		}
	}

	PythonModuleInstance::~PythonModuleInstance()
	{
	}

  void PythonModuleInstance::Load()
  {
  }

  void PythonModuleInstance::Compile()
  {
  }

	void PythonModuleInstance::Initialize () 
	{
	}

	void PythonModuleInstance::Destroy () 
	{
	}
}

