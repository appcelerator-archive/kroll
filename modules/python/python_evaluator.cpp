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

		std::string code = args[1]->ToString();

		// strip the beginning so we have some sense of tab normalization
		PythonEvaluator::ConvertLinefeeds(code);
		size_t startpos = code.find_first_not_of(" \t\n");
		if (std::string::npos != startpos)
			code = code.substr(startpos);

		PRINTD("Evaluating python source code:\n" << code);

		SharedBoundObject window_global = args.at(2)->ToObject();
		PyObject *py_context = PythonUtils::KObjectToPyObject(window_global);

		PyObject *main_module = PyImport_AddModule("__main__");
		PyObject *main_dict = PyModule_GetDict(main_module);

		// Insert the decorator function into globals
		SharedBoundMethod dec_fun = new DecoratorFunction(window_global);
		PyObject *py_dec_fun = PythonUtils::KMethodToPyObject(dec_fun);
		PyDict_SetItemString(main_dict, "expose", py_dec_fun);

		PyDict_SetItemString(main_dict, "window", py_context);
		PyObject *returnValue = PyRun_StringFlags(code.c_str(), Py_file_input, main_dict, main_dict, NULL);

		Py_DECREF(py_context);

		if (returnValue == NULL) 
		{
			 PythonUtils::ThrowException();
		}
		Py_DECREF(returnValue);
		return Value::Undefined;
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

	void PythonEvaluator::ConvertLinefeeds(std::string &code)
	{
		std::string::size_type loc = code.find("\r\n");
		while (loc != std::string::npos)
		{
			code.replace(loc, 2, "\n");
			loc = code.find("\r\n");
		}
	}

	DecoratorFunction::DecoratorFunction(SharedBoundObject window_global) :
		window_global(window_global)
	{
	}

	SharedValue DecoratorFunction::Call(const ValueList& args)
	{
		if (args.size() > 0 && args.at(0)->IsMethod())
		{
			SharedBoundMethod m = args.at(0)->ToMethod();
			SharedValue mv = Value::NewMethod(m);
			SharedValue fnamev = m->Get("__name__");
			const char* fname = fnamev->ToString();
			PRINTD("Set " << fname << " in window global");

			this->window_global->Set(fname, mv);
			return mv;
		}
		return Value::Undefined;
	}

	void DecoratorFunction::Set(const char *name, SharedValue value)
	{
	}

	SharedValue DecoratorFunction::Get(const char *name)
	{
		return Value::Undefined;
	}

	SharedStringList DecoratorFunction::GetPropertyNames()
	{
		return SharedStringList();
	}

