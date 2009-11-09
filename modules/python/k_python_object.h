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
		KPythonObject(PyObject *obj, bool readOnly);
		virtual ~KPythonObject();

		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual bool Equals(KObjectRef);
		virtual SharedStringList GetPropertyNames();
		PyObject* ToPython();

	private:
		PyObject *object;
		bool readOnly;
		KObjectRef delegate;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonObject);
	};
}
#endif

