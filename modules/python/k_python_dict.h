/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PYTHON_DICT_H_
#define _K_PYTHON_DICT_H_

#include "python_module.h"

namespace kroll
{
	class KPythonDict : public BoundObject
	{
	public:
		KPythonDict(PyObject *obj);
		virtual ~KPythonDict();

		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();

		PyObject* ToPython() { Py_INCREF(object); return object; }

	private:
		PyObject *object;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonDict);
	};
}
#endif

