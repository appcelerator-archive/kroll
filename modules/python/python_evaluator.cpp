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

		// Each script has it's own sub-interpreter
		PyThreadState* previous_state = PyThreadState_Get();
		PyThreadState* ts = Py_NewInterpreter();
		PyThreadState_Swap(ts);

		// Insert all the global properties into __builtins__
		SharedKObject window_global = args.at(2)->ToObject();
		PyObject* main_module = PyImport_AddModule("__main__");
		PyObject* globals = PyModule_GetDict(main_module);
		PyObject* builtins = PyDict_GetItemString(globals, "__builtins__");
		KObjectPropsToDict(window_global, builtins);

		// Run the script and retrieve the locals()
		PyObject *return_value = PyRun_String(
			code.c_str(),
			Py_file_input,
			globals,
			globals);
		PyObject* locals = PyRun_String("locals()", Py_eval_input, globals, globals);

		// Move all the new variables in locals() to  the
		// window context. These are things that are now defined.
		DictToKObjectProps(locals, window_global);

		/* TODO: Logging */
		SharedValue kv = Value::Undefined;
		if (return_value == NULL && PyErr_Occurred())
		{
			std::cout << "An error occured while parsing Python on the page: " << std::endl;
			PyErr_Print();
		}
		else
		{
			SharedValue kv = PythonUtils::ToKrollValue(return_value);
			Py_DECREF(return_value);
		}

		// Switch back to the global thread state
		PyThreadState_Swap(previous_state);

		return kv;
	}

	void PythonEvaluator::KObjectPropsToDict(SharedKObject o, PyObject* pyobj)
	{
		SharedStringList props = o->GetPropertyNames();
		for (size_t i = 0; i < props->size(); i++)
		{
			const char* k = props->at(i)->c_str();
			SharedValue v = o->Get(k);
			PyObject* pv = PythonUtils::ToPyObject(v);
			PyObject_SetAttrString(pyobj, k, pv);
			Py_DECREF(pv);
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
