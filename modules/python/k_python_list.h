/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PYTHON_LIST_H_
#define _K_PYTHON_LIST_H_

#include "python_module.h"

namespace kroll
{
	class KPythonList : public KList
	{
	public:
		KPythonList(PyObject *obj);
		virtual ~KPythonList();

		KValueRef Get(const char* name);
		void Set(const char* name, KValueRef value);
		virtual bool Equals(KObjectRef);
		SharedStringList GetPropertyNames();

		unsigned int Size();
		void Append(KValueRef value);
		virtual void SetAt(unsigned int index, KValueRef value);
		bool Remove(unsigned int index);
		KValueRef At(unsigned int index);

		PyObject* ToPython();

	protected:
		PyObject *list;
		AutoPtr<KPythonObject> object;
		DISALLOW_EVIL_CONSTRUCTORS(KPythonList);
	};
}
#endif

