/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PYTHON_OBJECT_H_
#define _K_PYTHON_OBJECT_H_

#include "python_module.h"

namespace kroll
{
	class KPythonObject : public BoundObject
	{
	public:
		KPythonObject(PyObject *obj);
		virtual ~KPythonObject();

		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();

		const PyObject* ToPython() { Py_INCREF(object); return object; }

	private:
		PyObject *object;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonObject);
	};
}
#endif

