/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PYTHON_METHOD_H_
#define _K_PYTHON_METHOD_H_

#include "python_module.h"

namespace kroll
{
	class KPythonObject;
	class KPythonMethod : public BoundMethod
	{
	public:
		KPythonMethod(PyObject *obj);
		virtual ~KPythonMethod();

		SharedValue Call(const ValueList& args);
		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		PyObject* ToPython();

	private:
		PyObject* method;
		SharedPtr<KPythonObject> object;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonMethod);
	};
}

#endif

