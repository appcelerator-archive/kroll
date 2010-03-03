/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "python_module.h"

namespace kroll
{
	PythonEvaluator::PythonEvaluator() :
		StaticBoundObject("Python.Evaluator")
	{
		/**
		 * @notiapi(method=True,name=Python.canEvaluate,since=0.7)
		 * @notiarg[String, mimeType] Code mime type
		 * @notiresult[bool] whether or not the mimetype is understood by Python
		 */
		SetMethod("canEvaluate", &PythonEvaluator::CanEvaluate);
		
		/**
		 * @notiapi(method=True,name=Python.evaluate,since=0.2) Evaluates a string as Python code
		 * @notiarg[String, mimeType] Code mime type (normally "text/python")
		 * @notiarg[String, name] name of the script source
		 * @notiarg[String, code] Python script code
		 * @notiarg[Object, scope] global variable scope
		 * @notiresult[Any] result of the evaluation
		 */
		SetMethod("evaluate", &PythonEvaluator::Evaluate);
	}
	
	void PythonEvaluator::CanEvaluate(const ValueList& args, KValueRef result)
	{
		args.VerifyException("canEvaluate", "s");
		
		result->SetBool(false);
		std::string mimeType = args.GetString(0);
		if (mimeType == "text/python")
		{
			result->SetBool(true);
		}
	}
	
	void PythonEvaluator::Evaluate(const ValueList& args, KValueRef result)
	{
		PyLockGIL lock;
		args.VerifyException("evaluate", "s s s o");

		//const char *mimeType = args.GetString(0).c_str();
		const char *name = args.GetString(1).c_str();
		std::string code = args.GetString(2);
		KObjectRef windowGlobal = args.GetObject(3);
		
		// Normalize tabs and convert line-feeds
		PythonEvaluator::Strip(code);
		PythonEvaluator::ConvertLineEndings(code);

		// Insert all the js global properties into a copy of globals()
		PyObject* mainModule = PyImport_AddModule("__main__");
		PyObject* globals = PyDict_Copy(PyModule_GetDict(mainModule));
		KObjectPropsToDict(windowGlobal, globals);

		// Another way to do this is to use a Python sub-interpreter,
		// but that seems to put us into restricted execution mode
		// sometimes. So we're going to try to isolate the variables
		// in this script by compiling it and supplying our own copy
		// of the globals.
		PyObject* compiled = Py_CompileStringFlags(code.c_str(), name, Py_file_input, NULL);
		if (compiled == NULL)
		{
			Logger *logger = Logger::Get("Python");
			std::string error("An error occured while parsing Python on the page: ");
			error.append(PythonUtils::PythonErrorToString());
			logger->Error(error);

			PyErr_Print();
			Py_DECREF(globals);
			result->SetUndefined();
			return;
		}

		PyObject *returnValue = PyEval_EvalCode((PyCodeObject*) compiled, globals, globals);

		// Clear the error indicator before doing anything else. It might
		// cause a a false positive for errors in other bits of Python.
		// TODO: Logging
		KValueRef kv = Value::Undefined;
		if (returnValue == NULL && PyErr_Occurred())
		{
			Logger *logger = Logger::Get("Python");
			std::string error("An error occured while parsing Python on the page: ");
			error.append(PythonUtils::PythonErrorToString());
			logger->Error(error);

			PyErr_Print();
		}
		else
		{
			KValueRef kv = PythonUtils::ToKrollValue(returnValue);
			Py_DECREF(returnValue);
		}

		// Move all the new variables in globals() to the window context.
		// These are things that are now defined globally in JS.
		DictToKObjectProps(globals, windowGlobal);
		result->SetValue(kv);
	}

	void PythonEvaluator::KObjectPropsToDict(KObjectRef o, PyObject* pyobj)
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
				KValueRef v = o->Get(k);
				PyObject* pv = PythonUtils::ToPyObject(v);
				PyDict_SetItemString(pyobj, k, pv);
				Py_DECREF(pv);
			}
		}
	}

	void PythonEvaluator::DictToKObjectProps(PyObject* dict, KObjectRef o)
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
				KValueRef newValue = PythonUtils::ToKrollValue(v);
				KValueRef existingValue = o->Get(sk.c_str());
				if (!newValue->Equals(existingValue))
				{
					o->Set(sk.c_str(), newValue);
				}
			}
			Py_DECREF(item);
		}
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
}
