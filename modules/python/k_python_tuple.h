/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PYTHON_TUPLE_H_
#define _K_PYTHON_TUPLE_H_

#include "python_module.h"

namespace kroll
{
	class KPythonTuple : public KList
	{
	public:
		KPythonTuple(PyObject *obj);
		virtual ~KPythonTuple();

		SharedValue Get(const char *name);
		void Set(const char *name, SharedValue value);
		virtual bool Equals(SharedKObject);
		SharedStringList GetPropertyNames();

		unsigned int Size();
		void Append(SharedValue value);
		virtual void SetAt(unsigned int index, SharedValue value);
		bool Remove(unsigned int index);
		SharedValue At(unsigned int index);

		PyObject* ToPython();

	protected:
		PyObject *tuple;
		SharedPtr<KPythonObject> object;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonTuple);
	};
}
#endif

