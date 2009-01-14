/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module.h"

namespace kroll
{
	const char * PythonStringToString(PyObject* value)
	{
		if (PyString_Check(value))
		{
			return PyString_AsString(value);
		}
		return "";
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

	void InitializeDefaultBindings (Host *host)
	{
		PyObject* mod = PyImport_ImportModule("__builtin__");

		if (mod)
		{
			// we bind the special module "api" to the global
			// variable defined in PRODUCT_NAME to give the
			// Python runtime access to it
			Value *api = host->GetGlobalObject()->Get("api");
			if (api->IsObject())
			{
				// we're going to clone the methods from api into our
				// own python scoped object
				StaticBoundObject *scope = ScopeMethodDelegate::CreateDelegate(host->GetGlobalObject(),api->ToObject());
				PyObject *pyapi = BoundObjectToPythonBoundObject(NULL,NULL,scope);
				PyObject_SetAttrString(mod,PRODUCT_NAME,pyapi);
				// now bind our new scope to python module
				Value *scopeRef = new Value(scope);
				host->GetGlobalObject()->Set((const char*)"python",scopeRef);
				KR_DECREF(scopeRef);
				// don't release the scope
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
		BoundMethod* method = static_cast<BoundMethod*>(p);
		KR_DECREF(method);
	}

	static PyObject *BoundMethodDispatcher (PyObject *s, PyObject *args)
	{
		void *sp = PyCObject_AsVoidPtr(s);
		BoundMethod *method = static_cast<BoundMethod*>(sp);
		ValueList a;
		try
		{
			for (int c=0;c<PyTuple_Size(args);c++)
			{
				PyObject *arg=PyTuple_GET_ITEM(args,c);
				a.push_back(PythonBoundObjectToValue(arg,NULL));
			}
			Value* result = method->Call(a);
			ScopedDereferencer r(result);
			return ValueToPythonBoundObject(result);
		}
		catch (Value *ex)
		{
			PyErr_SetObject(PyExc_Exception,ValueToPythonBoundObject(ex));
			Py_INCREF(Py_None);
			KR_DECREF(ex);
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

	PyObject* BoundMethodToPythonBoundObject(BoundMethod *method)
	{
		PyObject *self = PyCObject_FromVoidPtr(method,&PyDeleteBoundMethod);
		KR_ADDREF(method);
		return PyCFunction_New(&BoundMethodDispatcherDef, self);
	}


	PyObject* ValueToPythonBoundObject(Value* value)
	{
		if (value->IsBool())
		{
			return value->ToBool() ? Py_True : Py_False;
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
			BoundMethod *obj = value->ToMethod();
			if (typeid(obj) == typeid(PythonBoundMethod*))
			{
				return (PyObject*)((PythonBoundMethod*)obj)->ToPython();
			}
			return BoundMethodToPythonBoundObject(value->ToMethod());
		}
		if (value->IsObject())
		{
			BoundObject *obj = value->ToObject();
			if (typeid(obj) == typeid(PythonBoundObject*))
			{
				return (PyObject*)((PythonBoundObject*)obj)->ToPython();
			}
			return BoundObjectToPythonBoundObject(NULL,NULL,value->ToObject());
		}
		if (value->IsString())
		{
			return PyString_FromString(value->ToString().c_str());
		}
		if (value->IsDouble())
		{
			return PyFloat_FromDouble(value->ToDouble());
		}
		if (value->IsList())
		{
			BoundList *list = value->ToList();
			if (typeid(list) == typeid(PythonBoundList*))
			{
				return ((PythonBoundList*)list)->ToPython();
			}
			return BoundObjectToPythonBoundObject(NULL,NULL,list);
		}
		Py_INCREF(Py_None);
		return Py_None;
	}

	typedef struct {
	    PyObject_HEAD
		BoundObject *object;
	} PyBoundObject;

	static void PyBoundObject_dealloc(PyObject* self)
	{
		PyBoundObject *boundSelf = (PyBoundObject*)self;
		// std::cout << "PyBoundObject_dealloc called for " <<(void*)boundSelf << std::endl;
		KR_DECREF(boundSelf->object);
		PyObject_Del(self);
	}

	static PyObject* PyBoundObject_getattr(PyObject *self, char *name)
	{
		PyBoundObject *boundSelf = reinterpret_cast<PyBoundObject*>(self);
		// std::cout << "PyBoundObject_getattr called with " << name << " for " << (void*)boundSelf << std::endl;
		Value* result = boundSelf->object->Get(name);
		return ValueToPythonBoundObject(result);
	}

	static int PyBoundObject_setattr(PyObject *self, char *name, PyObject *value)
	{
		PyBoundObject *boundSelf = (PyBoundObject*)self;
		// std::cout << KR_FUNC << " called for " <<(void*)boundSelf << std::endl;
		Value* tiValue = PythonBoundObjectToValue(value,name);
		boundSelf->object->Set(name,tiValue);
		return 0;
	}

	static PyObject* PyBoundObject_tostring(PyObject *self)
	{
		PyBoundObject *boundSelf = (PyBoundObject*)self;
		// std::cout << KR_FUNC << " called for " <<(void*)boundSelf << std::endl;
		Value* result = boundSelf->object->Get("toString");
		if (result->IsMethod())
		{
			BoundMethod *method = result->ToMethod();
			ValueList args;
			Value* toString = method->Call(args);
			ScopedDereferencer r(toString);
			if (toString->IsString())
			{
				return PyString_FromString(toString->ToString().c_str());
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

	PyObject* BoundObjectToPythonBoundObject(PyObject* self, PyObject* args, BoundObject *bo)
	{
		//CHECK bo
		if (bo == NULL)
		{
			throw "BoundObject cannot be null";
		}
		PyBoundObject* obj;
		BoundList *list = dynamic_cast<BoundList*>(bo);
		if (list!=NULL)
		{
			// convert it into a list of wrapped Value objects
			PyObject* newlist = PyList_New(list->Size());
			for (int c = 0; c < list->Size(); c++)
			{
				Value* value = list->At(c);
				PyObject *item = ValueToPythonBoundObject(value);
				PyList_SetItem(newlist,c,item);
			}
			return newlist;
		}
		else
		{
			obj = PyObject_New(PyBoundObject, &PyBoundObjectType);
		}
		obj->object = bo;
		KR_ADDREF(obj->object);
		return (PyObject*)obj;
	}

	void ThrowPythonException()
	{
		PyObject *ptype, *pvalue, *trace;
		PyErr_Fetch(&ptype,&pvalue,&trace);
		PyErr_Print();
		PyErr_Clear();
		throw PythonBoundObjectToValue(pvalue,NULL);
	}

	Value* PythonBoundObjectToValue(PyObject* value, const char *name)
	{
		//FIXME - who is going to delete ref?


		if (Py_None==value)
		{
			return Value::Undefined();
		}
		if (PyString_Check(value))
		{
			std::string s = PythonStringToString(value);
			return new Value(s);
		}
		if (PyBool_Check(value))
		{
			return new Value(PythonBoolToBool(value));
		}
		if (PyInt_Check(value))
		{
			return new Value(PythonFixnumToInt(value));
		}
		if (PyFloat_Check(value))
		{
			return new Value(PythonFloatToDouble(value));
		}
		if (PyList_Check(value))
		{
			BoundList *l = new PythonBoundList(value);
			Value *til = new Value(l);
			return til;
		}
		if (PyClass_Check(value))
		{
			PythonBoundObject *v = new PythonBoundObject(value);
			Value* tiv = new Value(v);
			return tiv;
		}
		if (PyInstance_Check(value))
		{
			PythonBoundObject *v = new PythonBoundObject(value);
			Value* tiv = new Value(v);
			return tiv;
		}
		if (PyMethod_Check(value))
		{
			BoundMethod *m = new PythonBoundMethod(value,name);
			Value* tiv = new Value(m);
			return tiv;
		}
		if (PyFunction_Check(value))
		{
			BoundMethod *m = new PythonBoundMethod(value,name);
			Value* tiv = new Value(m);
			return tiv;
		}
		if (PyCallable_Check(value))
		{
			BoundMethod *m = new PythonBoundMethod(value,name);
			Value* tiv = new Value(m);
			return tiv;
		}
		if (PyObject_TypeCheck(value,&PyBoundObjectType))
		{
			PyBoundObject *o = reinterpret_cast<PyBoundObject*>(value);
			Value* tiv = new Value(o->object);
			return tiv;
		}

		std::cerr << "PythonBoundObjectToValue:nothing" << std::endl;
		PyObject_Print(value,stdout,0);
		printf("\n");

		return new Value();
	}
}


