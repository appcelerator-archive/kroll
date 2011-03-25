/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "python_module.h"

namespace kroll
{
    static PyObject* ContextToPyGlobals(KObjectRef context)
    {
		PyObject* mainModule = PyImport_AddModule("__main__");
		PyObject* globals = PyDict_Copy(PyModule_GetDict(mainModule));
		PyObject* builtins = PyDict_GetItemString(globals, "__builtins__");

		SharedStringList props = context->GetPropertyNames();
		for (size_t i = 0; i < props->size(); i++) {
			const char* k = props->at(i)->c_str();

			// Don't override builtin Python properties like open, etc. We *do* want
			// to override the PRODUCT_NAME object though, as each window contains a
			// version of that object with special delegated properties.
			if ((!PyDict_GetItemString(globals, k) && !PyObject_HasAttrString(builtins, k))
					|| !strcmp(k, PRODUCT_NAME))
			{
				KValueRef v = context->Get(k);
				PyObject* pv = PythonUtils::ToPyObject(v);
				PyDict_SetItemString(globals, k, pv);
				Py_DECREF(pv);
			}
		}

        return globals;
    }

    static void MergePyGlobalsWithContext(PyObject* globals, KObjectRef context)
    {
		// Avoid compiler warnings
		PyObject *items = PyObject_CallMethod(globals, (char*) "items", NULL);
		if (items == NULL)
			return;

		PyObject *iterator = PyObject_GetIter(items);
		if (iterator == NULL)
			return;

		PyObject *item;
		while ((item = PyIter_Next(iterator)))
		{
			PyObject* k = PyTuple_GetItem(item, 0);
			PyObject* v = PyTuple_GetItem(item, 1);
			std::string sk = PythonUtils::ToString(k);
			if (sk.find("__") != 0)
			{
				KValueRef newValue = PythonUtils::ToKrollValue(v);
				KValueRef existingValue = context->Get(sk.c_str());
				if (!newValue->Equals(existingValue))
				{
					context->Set(sk.c_str(), newValue);
				}
			}
			Py_DECREF(item);
		}
    }

	PythonInterpreter::PythonInterpreter()
	{
	}

    KValueRef PythonInterpreter::EvaluateFile(const char* filepath, KObjectRef context)
    {
        PyLockGIL lock;

        PyObject* globals = ContextToPyGlobals(context);
        FILE* file = fopen(filepath, "r");
        PyObject* pyResult = PyRun_FileEx(file, filepath, Py_file_input, globals, globals, 1);
        if (pyResult == NULL) {
            Py_DECREF(globals);
            std::string err = PythonUtils::PythonErrorToString();
            PyErr_Clear();
            throw ValueException::FromString(err);
        }

        MergePyGlobalsWithContext(globals, context);
        Py_DECREF(globals);

        KValueRef result = PythonUtils::ToKrollValue(pyResult);
        Py_DECREF(pyResult);
        return result;
    }
}
