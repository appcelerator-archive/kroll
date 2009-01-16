/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PYTHON_BOUND_OBJECT_H_
#define _PYTHON_BOUND_OBJECT_H_

#include "python_module.h"

namespace kroll
{
	class PythonBoundObject : public BoundObject
	{
	public:
		PythonBoundObject(PyObject *obj);
		virtual ~PythonBoundObject();

		virtual void Set(const char *name, SharedPtr<Value> value);
		virtual SharedPtr<Value> Get(const char *name);
		virtual SharedStringList GetPropertyNames();

		const PyObject* ToPython() { Py_INCREF(object); return object; }

	private:
		PyObject *object;
        DISALLOW_EVIL_CONSTRUCTORS(PythonBoundObject);
	};
}
#endif

