/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "python_module.h"

	SharedValue PythonEvaluator::Call(const ValueList& args)
	{
		PyLockGIL lock;

		if (args.size() != 3
			|| !args.at(1)->IsString()
			|| !args.at(2)->IsObject())
		{
			return Value::Undefined;
		}

		// Normalize tabs and convert line-feeds
		std::string code = args[1]->ToString();
		PythonEvaluator::Strip(code);
		PythonEvaluator::ConvertLineEndings(code);

		// Insert all the js global properties into a copy of globals()
		SharedKObject window_global = args.at(2)->ToObject();
		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* globals = PyDict_Copy(PyModule_GetDict(main_module));
		KObjectPropsToDict(window_global, globals);

		// Another way to do this is to use a Python sub-interpreter,
		// but that seems to put us into restricted execution mode
		// sometimes. So we're going to try to isolate the variables
		// in this script by compiling it and supplying our own copy
		// of the globals.
		PyObject* compiled = Py_CompileStringFlags(code.c_str(), "<window>", Py_file_input, NULL);
		if (compiled == NULL)
		{
			Logger *logger = Logger::Get("Python");
			std::string error("An error occured while parsing Python on the page: ");
			error.append(PythonUtils::PythonErrorToString());
			logger->Error(error);

			PyErr_Print();
			Py_DECREF(globals);
			return Value::Undefined;
		}

		PyObject *return_value = PyEval_EvalCode((PyCodeObject*) compiled, globals, globals);

		// Clear the error indicator before doing anything else. It might
		// cause a a false positive for errors in other bits of Python.
		// TODO: Logging
		SharedValue kv = Value::Undefined;
		if (return_value == NULL && PyErr_Occurred())
		{
			Logger *logger = Logger::Get("Python");
			std::string error("An error occured while parsing Python on the page: ");
			error.append(PythonUtils::PythonErrorToString());
			logger->Error(error);

			PyErr_Print();
		}
		else
		{
			SharedValue kv = PythonUtils::ToKrollValue(return_value);
			Py_DECREF(return_value);
		}

		// Move all the new variables in globals() to the window context.
		// These are things that are now defined globally in JS.
		DictToKObjectProps(globals, window_global);
		return kv;
	}

	void PythonEvaluator::KObjectPropsToDict(SharedKObject o, PyObject* pyobj)
	{
		PyObject* builtins = PyDict_GetItemString(pyobj, "__builtins__");

		SharedStringList props = o->GetPropertyNames();
		for (size_t i = 0; i < props->size(); i++)
		{
			const char* k = props->at(i)->c_str();

			// Don't override builtin Python properties like open, etc. We *do* want
			// to override the PRODUCT_NAME object though, as each window contains a
			// version of that object with special delegated properties.
			if ((!PyDict_GetItemString(pyobj, k) && !PyObject_HasAttrString(builtins, k))
					|| !strcmp(k, PRODUCT_NAME))
			{
				SharedValue v = o->Get(k);
				PyObject* pv = PythonUtils::ToPyObject(v);
				PyDict_SetItemString(pyobj, k, pv);
				Py_DECREF(pv);
			}
		}
	}

	void PythonEvaluator::DictToKObjectProps(PyObject* dict, SharedKObject o)
	{
		// Avoid compiler warnings
		PyObject *items = PyObject_CallMethod(dict, (char*) "items", NULL);
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
				SharedValue newValue = PythonUtils::ToKrollValue(v);
				SharedValue existingValue = o->Get(sk.c_str());
				if (!newValue->Equals(existingValue))
				{
					o->Set(sk.c_str(), newValue);
				}
			}
			Py_DECREF(item);
		}
	}

	void PythonEvaluator::Set(const char *name, SharedValue value)
	{
	}

	SharedValue PythonEvaluator::Get(const char *name)
	{
		return Value::Undefined;
	}

	SharedStringList PythonEvaluator::GetPropertyNames()
	{
		return SharedStringList();
	}

	void PythonEvaluator::Strip(std::string &code)
	{
		size_t startpos = code.find_first_not_of(" \t\n");
		if (std::string::npos != startpos)
			code.replace(0, startpos, "");

		startpos = code.find_last_not_of(" \t\n") + 1;
		if (startpos != code.size())
			code.replace(startpos, code.size() - startpos, "\n");
	}

	void PythonEvaluator::ConvertLineEndings(std::string &code)
	{
		std::string::size_type loc = code.find("\r\n");
		while (loc != std::string::npos)
		{
			code.replace(loc, 2, "\n");
			loc = code.find("\r\n");
		}
	}
