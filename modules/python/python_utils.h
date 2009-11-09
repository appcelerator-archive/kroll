/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef PYTHON_TYPES_H_
#define PYTHON_TYPES_H_

#include <typeinfo>
#include "python_module.h"

namespace kroll
{
	class PythonUtils
	{
	public:
		static void InitializePythonKClasses();
		static KValueRef ToKrollValue(PyObject* value);
		static PyObject* ToPyObject(KValueRef value);
		static PyObject* ToPyObject(const ValueList& list);
		static const char* ToString(PyObject* value);
		static PyObject* KObjectToPyObject(KValueRef o);
		static PyObject* KMethodToPyObject(KValueRef o);
		static PyObject* KListToPyObject(KValueRef o);
		static std::string PythonErrorToString();

	private:
		PythonUtils() {}
		~PythonUtils () {}
	};
}

#endif
