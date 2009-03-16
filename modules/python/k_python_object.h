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
	class KPythonObject : public KObject
	{
	public:
		KPythonObject(PyObject *obj);
		KPythonObject(PyObject *obj, bool read_only);
		virtual ~KPythonObject();

		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		PyObject* ToPython();

	private:
		PyObject *object;
		bool read_only;
		SharedKObject delegate;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonObject);
	};
}
#endif

