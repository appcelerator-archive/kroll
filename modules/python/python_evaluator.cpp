/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "python_module.h"

	SharedValue PythonEvaluator::Call(const ValueList& args)
	{

		if (args.size() != 3
			|| !args.at(1)->IsString()
			|| !args.at(2)->IsObject())
		{
			return Value::Undefined;
		}

		// Normalize tabs and convert line-feeds
		std::string code = args[1]->ToString();
		PythonEvaluator::StripLeadingWhitespace(code);
		PythonEvaluator::ConvertLineEndings(code);

		// Copy __main__'s globals() and insert all window global
		// properties into it, so that we don't pollute Python's
		// __main__ globals()
		SharedBoundObject window_global = args.at(2)->ToObject();
		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* main_dict = PyModule_GetDict(main_module);
		PyObject* my_globals = PyDict_Copy(main_dict);
		KObjectPropsToDict(window_global, my_globals);

		// Execute the script using a blank dictionary as locals()
		PyObject* my_locals = PyDict_New();
		PyObject *return_value = PyRun_StringFlags(
			code.c_str(),
			Py_file_input,
			my_globals,
			my_locals,
			NULL);

		/* TODO: Logging */
		if (return_value == NULL && PyErr_Occurred())
		{
			std::cout << "An error occured while parsing Python on the page: " << std::endl;
			PyErr_Print();
		}
		else
		{
			Py_DECREF(return_value);
		}

		// Move all the new variables in locals() (my_locals) to
		// the window context. These are things that are now defined.
		DictToKObjectProps(my_locals, window_global);
		Py_DECREF(my_locals);
		Py_DECREF(my_globals);

		return Value::Undefined;
	}

	void PythonEvaluator::KObjectPropsToDict(SharedBoundObject o, PyObject* map)
	{
		SharedStringList props = o->GetPropertyNames();
		for (size_t i = 0; i < props->size(); i++)
		{
			const char* k = props->at(i)->c_str();
			SharedValue v = o->Get(k);
			PyObject* pv = PythonUtils::ToPyObject(v);
			PyDict_SetItemString(map, k, pv);
			Py_DECREF(pv);
		}
	}

	void PythonEvaluator::DictToKObjectProps(PyObject* dict, SharedBoundObject o)
	{
		// Avoid compiler warnings
		PyObject *items = PyObject_CallMethod(dict, (char*) "items", NULL);
		if (items == NULL)
			return;

		PyObject *iterator = PyObject_GetIter(items);
		if (iterator == NULL)
			return;

		PyObject *item;
		while ((item = PyIter_Next(iterator))) {
			PyObject* k = PyTuple_GetItem(item, 0);
			PyObject* v = PyTuple_GetItem(item, 1);
			std::string sk = PythonUtils::ToString(k);

			if (sk.find("__") != 0)
			{
				SharedValue kv = PythonUtils::ToKrollValue(v);
				o->Set(sk.c_str(), kv);
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

	void PythonEvaluator::StripLeadingWhitespace(std::string &code)
	{
		size_t startpos = code.find_first_not_of(" \t\n");
		if (std::string::npos != startpos)
			code.replace(0, startpos, "");
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
