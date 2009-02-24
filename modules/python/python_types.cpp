/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module.h"

namespace kroll
{
	SharedBoundObject PythonUtils::scope;

	const char * PythonUtils::ToString(PyObject* value)
	{
		if (PyString_Check(value))
		{
			return PyString_AsString(value);
		}
		PyObject *o = PyObject_Str(value);
		const char *result = PythonUtils::ToString(o);
		Py_DECREF(o);
		return result;
	}

	bool PythonBoolToBool(PyObject* value)
	{
		if (PyBool_Check(value))
		{
			return PyObject_IsTrue(value);
		}
		return false;
	}

	int PythonFixnumToInt(PyObject* value)
	{
		if (PyInt_Check(value))
		{
			return (int)PyInt_AsLong(value);
		}
		return 0;
	}

	double PythonFloatToDouble(PyObject* value)
	{
		if (PyFloat_Check(value))
		{
			return PyFloat_AsDouble(value);
		}
		return 0.0;
	}
	
	void ConvertLinefeeds(std::string &code)
	{
		while (1)
		{
			std::string::size_type loc = code.find("\r\n");
			if (loc == std::string::npos) break;
			code.replace(loc,2,"\n");
		}
	}

	class PythonEvaluator : public BoundMethod
	{
	public:
		virtual SharedValue Call(const ValueList& args) {
			if (args.size() == 3 && args[1]->IsString()) {

				// strip the beginning so we have some sense of tab normalization
				std::string code = args[1]->ToString();
				ConvertLinefeeds(code);
				size_t startpos = code.find_first_not_of(" \t\n");
				if (std::string::npos != startpos)
					code = code.substr(startpos);

#ifdef DEBUG
				std::cout << "Evaluating python source code:\n" << code << std::endl;
#endif
				SharedBoundObject context = args[2]->ToObject();
				PyObject *py_context = PythonUtils::ToObject(NULL, NULL, context);

				PyObject *main_module = PyImport_AddModule("__main__");
				PyObject *main_dict = PyModule_GetDict(main_module);

				PyDict_SetItemString(main_dict, "window", py_context);
				PyObject *returnValue = PyRun_StringFlags(code.c_str(), Py_file_input, main_dict, main_dict, NULL);

				Py_DECREF(py_context);

				if (returnValue == NULL) 
				{
					 PythonUtils::ThrowException();
				}
				Py_DECREF(returnValue);
			}
			return Value::Null;
		}

		virtual void Set(const char *name, SharedValue value) {}
		virtual SharedValue Get(const char *name) { return Value::Null; }
		virtual SharedStringList GetPropertyNames() { return SharedStringList(); }
	};

	SharedBoundMethod PythonUtils::evaluator = SharedBoundMethod(new PythonEvaluator());

	void PythonUtils::InitializeDefaultBindings (Host *host)
	{
		PyObject* mod = PyImport_ImportModule("__builtin__");

		if (mod)
		{
			// we bind the special module "api" to the global
			// variable defined in PRODUCT_NAME to give the
			// Python runtime access to it
			SharedValue api = host->GetGlobalObject()->Get("API");
			if (api->IsObject())
			{
				// we're going to clone the methods from api into our
				// own python scoped object
				SharedBoundObject hostobj = host->GetGlobalObject();
				SharedBoundObject apiobj = api->ToObject();
				scope = ScopeMethodDelegate::CreateDelegate(hostobj, apiobj);
				scope->Set("evaluate", Value::NewMethod(evaluator));
				PyObject *pyapi = PythonUtils::ToObject(NULL,NULL,hostobj);
				printf("binding %s into Python __builtin__\n", PRODUCT_NAME);
				PyObject_SetAttrString(mod,PRODUCT_NAME,pyapi);
				// now bind our new scope to python module
				SharedValue scopeRef = Value::NewObject(scope);
				host->GetGlobalObject()->Set((const char*)"Python",scopeRef);
				// don't release the scope
			}
			else
			{
				std::cerr << "! Couldn't find API module to bind Python module to" << std::endl;
			}
			Py_DECREF(mod);
		}
	}
	// PyObject* ValueListToPythonArray(const ValueList& list)
	// {
	// 	int size = list.size();
	//
	// 	if (size == 0)
	// 	{
	// 		Py_INCREF(Py_None);
	// 		return Py_None;
	// 	}
	//
	// 	PyObject *array = PyTuple_New(size);
	// 	for (int c=0;c<size;c++)
	// 	{
	// 		Value *value = list.at(c);
	// 		PyTuple_SET_ITEM(array,c,ValueToPythonBoundObject(value));
	// 	}
	// 	return array;
	// }
	static void PyDeleteBoundMethod(void *p)
	{
		// std::cout << "PyDeleteBoundMethod being called for " << p << std::endl;
		SharedBoundMethod* method = static_cast< SharedBoundMethod* >(p);
		delete method;
	}

	static PyObject *BoundMethodDispatcher (PyObject *s, PyObject *args)
	{
		void *sp = PyCObject_AsVoidPtr(s);
		SharedBoundMethod* method = static_cast<SharedBoundMethod*>(sp);
		ValueList a;
		// std::cout << "calling BoundMethodDispatcher for "  << sp << std::endl;
		Py_INCREF(s);
		try
		{
			for (int c=0;c<PyTuple_Size(args);c++)
			{
				PyObject *arg=PyTuple_GET_ITEM(args,c);
				SharedValue argument = PythonUtils::ToValue(arg,NULL);
				a.push_back(argument);
			}
			SharedBoundMethod m = (*method);
			SharedValue result = m->Call(a);
			Py_DECREF(s);
			return PythonUtils::ToObject(result);
		}
		catch (SharedValue ex)
		{
			PyErr_SetObject(PyExc_Exception,PythonUtils::ToObject(ex));
			Py_INCREF(Py_None);
			Py_DECREF(s);
			return Py_None;
		}
	}


	static PyMethodDef BoundMethodDispatcherDef =
	{
			"BoundMethodDispatcher",
			&BoundMethodDispatcher,
			METH_VARARGS,
			"dispatcher for BoundMethod"
	};

	PyObject* PythonUtils::ToObject(SharedBoundMethod method)
	{
		SharedBoundMethod *m = new SharedBoundMethod(method);
		PyObject *self = PyCObject_FromVoidPtr(m,&PyDeleteBoundMethod);
		return PyCFunction_New(&BoundMethodDispatcherDef, self);
	}


	PyObject* PythonUtils::ToObject(SharedValue value)
	{
		if (value->IsBool())
		{
			return value->ToBool() ? Py_True : Py_False;
		}
		if (value->IsDouble())
		{
			return PyFloat_FromDouble(value->ToDouble());
		}
		if (value->IsInt())
		{
			return PyInt_FromLong(value->ToInt());
		}
		if (value->IsNull() || value->IsUndefined())
		{
			Py_INCREF(Py_None);
			return Py_None;
		}
		if (value->IsMethod())
		{
			SharedBoundMethod obj = value->ToMethod();
			if (typeid(obj.get()) == typeid(PythonBoundMethod*))
			{
				return (PyObject*)((PythonBoundMethod*)obj.get())->ToPython();
			}
			return PythonUtils::ToObject(value->ToMethod());
		}
		if (value->IsList())
		{
			SharedBoundList list = value->ToList();
			if (typeid(list.get()) == typeid(PythonBoundList*))
			{
				return ((PythonBoundList*)list.get())->ToPython();
			}
			return PythonUtils::ToObject(NULL,NULL,list);
		}
		if (value->IsObject())
		{
			SharedBoundObject obj = value->ToObject();
			if (typeid(obj.get()) == typeid(PythonBoundObject*))
			{
				return (PyObject*)((PythonBoundObject*)obj.get())->ToPython();
			}
			return PythonUtils::ToObject(NULL,NULL,value->ToObject());
		}
		if (value->IsString())
		{
			return PyString_FromString(value->ToString());
		}
		Py_INCREF(Py_None);
		return Py_None;
	}

	typedef struct {
		PyObject_HEAD
		SharedBoundObject* object;
	} PyBoundObject;

	static void PyBoundObject_dealloc(PyObject* self)
	{
		PyBoundObject *boundSelf = reinterpret_cast<PyBoundObject*>(self);
		delete boundSelf->object;
		PyObject_Del(self);
	}

	static PyObject* PyBoundObject_getattr(PyObject *self, char *name)
	{
		PyBoundObject *boundSelf = reinterpret_cast<PyBoundObject*>(self);
		Py_INCREF(boundSelf);
		SharedValue result = boundSelf->object->get()->Get(name);
		Py_DECREF(boundSelf);
		return PythonUtils::ToObject(result);
	}

	static int PyBoundObject_setattr(PyObject *self, char *name, PyObject *value)
	{
		PyBoundObject *boundSelf = reinterpret_cast<PyBoundObject*>(self);
		Py_INCREF(boundSelf);
		SharedValue tiValue = PythonUtils::ToValue(value,name);
		boundSelf->object->get()->Set(name, tiValue);
		Py_DECREF(boundSelf);
		return 0;
	}

	static PyObject* PyBoundObject_tostring(PyObject *self)
	{
		PyBoundObject *boundSelf = reinterpret_cast<PyBoundObject*>(self);
		Py_INCREF(boundSelf);
		SharedValue result = boundSelf->object->get()->Get("toString");
		Py_DECREF(boundSelf);
		if (result->IsMethod())
		{
			SharedBoundMethod method = result->ToMethod();
			ValueList args;
			SharedValue toString = method->Call(args);
			if (toString->IsString())
			{
				return PyString_FromString(toString->ToString());
			}
		}
		char str[255];
		sprintf(str,"<BoundObject %x>",(unsigned int)self);
		return PyString_FromString(str);
	}

	static PyTypeObject PyBoundObjectType =
	{
	    PyObject_HEAD_INIT(NULL)
	    0,
	    "BoundObject",
	    sizeof(PyBoundObject),
	    0,
	    PyBoundObject_dealloc, 		/*tp_dealloc*/
	    0,          	 			/*tp_print*/
	    PyBoundObject_getattr,    	/*tp_getattr*/
	    PyBoundObject_setattr,    	/*tp_setattr*/
	    0,          	 			/*tp_compare*/
	    0,          	 			/*tp_repr*/
	    0,          	 			/*tp_as_number*/
	    0,    						/*tp_as_sequence*/
	    0,     						/*tp_as_mapping*/
	    0,           	 			/*tp_hash */
		0,							/*tp_call */
		PyBoundObject_tostring,		/*tp_str */
		0,							/*tp_getattro*/
		0,							/*tp_setattro*/
		0,							/*tp_as_buffer*/
		0,							/*tp_flags*/
		0,							/*tp_doc*/
	};

	PyObject* PythonUtils::ToObject(PyObject* self, PyObject* args, SharedBoundObject bo)
	{
		//CHECK bo
		if (bo.isNull())
		{
			throw "BoundObject cannot be null";
		}
		SharedBoundList list = bo.cast<BoundList>();
		if (!list.isNull())
		{
			// convert it into a list of wrapped Value objects
			PyObject* newlist = PyList_New(list->Size());
			for (int c = 0; c < list->Size(); c++)
			{
				SharedValue value = list->At(c);
				PyObject *item = PythonUtils::ToObject(value);
				PyList_SetItem(newlist,c,item);
			}
			return newlist;
		}
		else
		{
			PyBoundObject* obj = PyObject_New(PyBoundObject, &PyBoundObjectType);
			SharedBoundObject* bo_ptr = new SharedBoundObject(bo);
			obj->object = bo_ptr;
			return (PyObject*)obj;
		}
	}

	void PythonUtils::ThrowException()
	{
		PyObject *ptype, *pvalue, *trace;
		PyErr_Fetch(&ptype,&pvalue,&trace);
		PyErr_Print();
		const char *err = PythonUtils::ToString(pvalue);
		ValueException ex = ValueException::FromString(err);
		Py_XDECREF(ptype);
		Py_XDECREF(pvalue);
		Py_XDECREF(trace);
		PyErr_Clear();
		throw ex;
	}

	SharedValue PythonUtils::ToValue(PyObject* value, const char *name)
	{
		//FIXME - who is going to delete ref?


		if (Py_None==value)
		{
			return Value::Undefined;
		}
		if (PyString_Check(value))
		{
			std::string s = PythonUtils::ToString(value);
			return Value::NewString(s);
		}
		if (PyBool_Check(value))
		{
			return Value::NewBool(PythonBoolToBool(value));
		}
		if (PyInt_Check(value))
		{
			return Value::NewInt(PythonFixnumToInt(value));
		}
		if (PyFloat_Check(value))
		{
			return Value::NewDouble(PythonFloatToDouble(value));
		}
		if (PyList_Check(value))
		{
			SharedBoundList l = new PythonBoundList(value);
			SharedValue til = Value::NewList(l);
			return til;
		}
		if (PyClass_Check(value))
		{
			SharedBoundObject v = new PythonBoundObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}
		if (PyInstance_Check(value))
		{
			SharedBoundObject v = new PythonBoundObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}
		if (PyMethod_Check(value))
		{
			SharedBoundMethod m = new PythonBoundMethod(value,name);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		if (PyFunction_Check(value))
		{
			SharedBoundMethod m = new PythonBoundMethod(value,name);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		if (PyCallable_Check(value))
		{
			SharedBoundMethod m = new PythonBoundMethod(value,name);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		if (PyObject_TypeCheck(value,&PyBoundObjectType))
		{
			PyBoundObject *o = reinterpret_cast<PyBoundObject*>(value);
			SharedValue tiv = Value::NewObject(*(o->object));
			return tiv;
		}

		std::cerr << "PythonBoundObjectToValue:nothing" << std::endl;
		PyObject_Print(value,stdout,0);
		printf("\n");

		return new Value();
	}
}


