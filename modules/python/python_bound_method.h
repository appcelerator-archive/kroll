/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PYTHON_BOUND_METHOD_H_
#define _PYTHON_BOUND_METHOD_H_

#include "python_module.h"

namespace kroll
{
	class PythonBoundObject;
	class PythonBoundMethod : public BoundMethod
	{
	public:
		PythonBoundMethod(PyObject *obj, const char *name);
		virtual ~PythonBoundMethod();

		SharedValue Call(const ValueList& args);
		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();

		const PyObject* ToPython() { Py_INCREF(object); return object; }

	private:
		char* name;
		PyObject* object;
		SharedPtr<BoundObject> delegate;
        DISALLOW_EVIL_CONSTRUCTORS(PythonBoundMethod);
	};
}

#endif

