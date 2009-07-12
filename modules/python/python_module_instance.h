/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PYTHON_MODULE_INSTANCE_H
#define _PYTHON_MODULE_INSTANCE_H

#include "python_module.h"

#ifdef OS_WIN32
#define PATH_SEPARATOR ";"
#else
#define PATH_SEPARATOR ":"
#endif

namespace kroll
{
	class PythonModuleInstance : public Module
	{
	public:
		PythonModuleInstance(Host *host, std::string path, std::string dir, std::string name);
	protected:
		virtual ~PythonModuleInstance();
	public:
		void Initialize ();
		void Destroy ();

    void AppendPath();
    void Load();
	private:
    PyObject* module;
		std::string path;
    std::string dir;
    std::string name;
        DISALLOW_EVIL_CONSTRUCTORS(PythonModuleInstance);
	};
}

#endif
