/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module.h"

namespace kroll
{
	SharedBoundObject PythonUtils::scope;
	PyObject* PythonUtils::ToPyObject(SharedValue value)
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
			if (typeid(obj.get()) == typeid(KPythonMethod*))
			{
				return (PyObject*)((KPythonMethod*)obj.get())->ToPython();
			}
			return PythonUtils::KMethodToPyObject(value->ToMethod());
		}
		if (value->IsList())
		{
			SharedBoundList list = value->ToList();
			if (typeid(list.get()) == typeid(KPythonList*))
			{
				return ((KPythonList*)list.get())->ToPython();
			}
			return PythonUtils::KObjectToPyObject(list);
		}
		if (value->IsObject())
		{
			SharedBoundObject obj = value->ToObject();
			if (typeid(obj.get()) == typeid(KPythonObject*))
			{
				return (PyObject*)((KPythonObject*)obj.get())->ToPython();
			}
			return PythonUtils::KObjectToPyObject(value->ToObject());
		}
		if (value->IsString())
		{
			return PyString_FromString(value->ToString());
		}
		Py_INCREF(Py_None);
		return Py_None;
	}

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
	// 		PyTuple_SET_ITEM(array,c,ValueToKPythonObject(value));
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
				SharedValue argument = PythonUtils::ToKrollValue(arg,NULL);
				a.push_back(argument);
			}
			SharedBoundMethod m = (*method);
			SharedValue result = m->Call(a);
			Py_DECREF(s);
			return PythonUtils::ToPyObject(result);
		}
		catch (SharedValue ex)
		{
			PyErr_SetObject(PyExc_Exception,PythonUtils::ToPyObject(ex));
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

	PyObject* PythonUtils::KMethodToPyObject(SharedBoundMethod method)
	{
		SharedBoundMethod *m = new SharedBoundMethod(method);
		PyObject *self = PyCObject_FromVoidPtr(m,&PyDeleteBoundMethod);
		return PyCFunction_New(&BoundMethodDispatcherDef, self);
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
		return PythonUtils::ToPyObject(result);
	}

	static int PyBoundObject_setattr(PyObject *self, char *name, PyObject *value)
	{
		PyBoundObject *boundSelf = reinterpret_cast<PyBoundObject*>(self);
		Py_INCREF(boundSelf);
		SharedValue tiValue = PythonUtils::ToKrollValue(value,name);
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
		sprintf(str,"<BoundObject %lx>",(unsigned long)self);
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

	PyObject* PythonUtils::KObjectToPyObject(SharedBoundObject bo)
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
			for (unsigned int c = 0; c < list->Size(); c++)
			{
				SharedValue value = list->At(c);
				PyObject *item = PythonUtils::ToPyObject(value);
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

	SharedValue PythonUtils::ToKrollValue(PyObject* value, const char *name)
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
			SharedBoundList l = new KPythonList(value);
			SharedValue til = Value::NewList(l);
			return til;
		}
		if (PyClass_Check(value))
		{
			SharedBoundObject v = new KPythonObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}
		if (PyInstance_Check(value))
		{
			SharedBoundObject v = new KPythonObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}
		if (PyMethod_Check(value))
		{
			SharedBoundMethod m = new KPythonMethod(value,name);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		if (PyFunction_Check(value))
		{
			SharedBoundMethod m = new KPythonMethod(value,name);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		if (PyCallable_Check(value))
		{
			SharedBoundMethod m = new KPythonMethod(value,name);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		if (PyObject_TypeCheck(value,&PyBoundObjectType))
		{
			PyBoundObject *o = reinterpret_cast<PyBoundObject*>(value);
			SharedValue tiv = Value::NewObject(*(o->object));
			return tiv;
		}

		std::cerr << "KPythonObjectToKrollValue:nothing" << std::endl;
		PyObject_Print(value,stdout,0);
		printf("\n");

		return new Value();
	}
}


