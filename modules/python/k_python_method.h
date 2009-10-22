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
	class KPythonMethod : public KMethod
	{
	public:
		KPythonMethod(PyObject *obj);
		virtual ~KPythonMethod();

		KValueRef Call(const ValueList& args);
		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual bool Equals(KObjectRef);
		virtual SharedStringList GetPropertyNames();
		PyObject* ToPython();

	private:
		PyObject* method;
		AutoPtr<KPythonObject> object;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonMethod);
	};
}

#endif

