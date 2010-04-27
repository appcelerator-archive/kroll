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
		SetMethod("canEvaluate", &PythonEvaluator::CanEvaluate);
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
		
		// We must unindent the code block to prevent parsing errors.
		PythonEvaluator::UnindentCode(code);

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

	void PythonEvaluator::UnindentCode(std::string &code)
	{
		// Determine indent width of the code block
		size_t startOffset = code.find_first_not_of("\r\n");  // ignore any newlines at start of block
		size_t blockIndent = code.find_first_not_of(" \t", startOffset) - startOffset;
		if (blockIndent == 0) return;

		std::string dirtyCode = code.substr(startOffset);
		code.clear();

		// Process code line by line unindenting code
		Poco::StringTokenizer lines(dirtyCode, "\n");
		Poco::StringTokenizer::Iterator i;
		for (i = lines.begin(); i != lines.end(); i++)
		{
			std::string line = *i;
			if (line.length() > 0)
			{
				size_t lineIndent = line.find_first_not_of(" \t");

				if (lineIndent == std::string::npos)
				{
					// Allow lines with only whitespace to pass through.
					code.append(line);
				}
				else if (lineIndent < blockIndent)
				{
					// If the line has a smaller indent than the block, warn the user!
					throw ValueException::FromFormat("Indentation error in script: %s", line.c_str());
				}
				else
				{
					code.append(line, blockIndent, line.length() - blockIndent);
				}
			}
			code.append(1, '\n');
		}
	}
}
