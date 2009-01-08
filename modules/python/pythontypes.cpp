/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "pythontypes.h"
#include "pythonmodule.h"
#include "pythonvalue.h"
#include "pythonmethod.h"

static PyObject* Bind(PyObject *self, PyObject *args)
{
	std::cout << "Python Bind called" << std::endl;
	char *name;
	PyObject *object;
	if (PyArg_ParseTuple(args,"sO",&name,&object))
	{
		std::cout << "binding python object: " << name << std::endl;
		kroll::PythonModule *python_module = kroll::PythonModule::Instance();
		Value* v = kroll::PythonValueToValue(object,name);
		python_module->GetHost()->GetGlobalObject()->Set(name,v);
	}
	//FIXME - error handling
	Py_INCREF(Py_None);
    return Py_None;
}

namespace kroll
{
	std::string PythonStringToString(PyObject* value)
	{
		if (PyString_Check(value))
		{
			return std::string(PyString_AsString(value));
		}
		return std::string();
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

	//PyObject* wrapper_class;

	// static PyObject*
	// BoundObjectWrapper_MethodMissing(int argc, PyObject* *argv, PyObject* self) {
	// 	// BoundObject* value = NULL;
	// 	// Data_Get_Struct(self, BoundObject, value);
	// 	//
	// 	// if (value != NULL) {
	// 	// 	PyObject* method = rb_funcall(*argv, rb_intern("to_s"), 0);
	// 	// 	method = rb_funcall(method, rb_intern("downcase"), 0);
	// 	// 	const char *method_name = StringValueCStr(method);
	// 	//
	// 	// 	ArgList args;
	// 	// 	for (int i = 1; i < argc; i++) {
	// 	// 		args.push_back(PythonValueToValue(argv[i]));
	// 	// 	}
	// 	// 	ReturnValue *returnValue = value->Call(method_name, args);
	// 	// }
	// 	// return Qnil;
	// 	return 0;
	// }

	static PyMethodDef meth[] = {
		{"tiBind", &Bind, METH_VARARGS, "bind an object"},
		{NULL}
	};

	void InitializeDefaultBindings ()
	{
		PyObject* mod = PyImport_ImportModule("__builtin__");

		if (mod)
		{
			for (int i = 0; i < 1; ++i)
			{
				std::cout << "+++ binding: " << meth[i].ml_name << std::endl;
				PyObject_SetAttrString(mod, meth[i].ml_name, PyCFunction_New(&meth[i], NULL));
			}

			Py_DECREF(mod);
		}
	}


	PyObject* ValueListToPythonArray(const ValueList& list)
	{

		// PyObject* array = rb_ary_new2(list.size());
		// ArgList::const_iterator iter;
		// for (iter = list.begin(); iter != list.end(); iter++)
		// {
		// 	ArgValue value = *iter;
		// 	rb_ary_concat(array, ValueToPythonValue(value));
		// }
		// return array;
		return 0;
	}
	static void PyDeleteBoundMethod(void *p)
	{
		BoundMethod* method = static_cast<BoundMethod*>(p);
		KR_DECREF(method);
	}

	static PyObject *BoundMethodDispatcher (PyObject *s, PyObject *args)
	{
		void *sp = PyCObject_AsVoidPtr(s);
		BoundMethod *method = static_cast<BoundMethod*>(sp);
		std::cout << "method => " << (void*)method << " with " << PyTuple_Size(args) << " args" << std::endl;
		ValueList a;
		try
		{
			for (int c=0;c<PyTuple_Size(args);c++)
			{
				PyObject *arg=PyTuple_GET_ITEM(args,c);
				a.push_back(PythonValueToValue(arg,NULL));
			}
			Value* result = method->Call(a,NULL);
			std::cout << "result = " << result->ToTypeString() << std::endl;
			return ValueToPythonValue(result);
		}
		catch (Value *ex)
		{
			PyErr_SetObject(PyExc_Exception,ValueToPythonValue(ex));
			Py_INCREF(Py_None);
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

	PyObject* BoundMethodToPythonValue(BoundMethod *method)
	{
		PyObject *self = PyCObject_FromVoidPtr(method,&PyDeleteBoundMethod);
		KR_ADDREF(method);
		return PyCFunction_New(&BoundMethodDispatcherDef, self);
	}


	PyObject* ValueToPythonValue(Value* value)
	{
		if (value->IsBool()) {
			return value->ToBool() ? Py_True : Py_False;
		}
		if (value->IsInt()) {
			return PyInt_FromLong(value->ToInt());
		}
		if (value->IsNull() || value->IsUndefined()) {
			Py_INCREF(Py_None);
			return Py_None;
		}
		if (value->IsMethod()) {
			BoundMethod *obj = value->ToMethod();
			if (typeid(obj) == typeid(PythonMethod*))
			{
				return ((PythonMethod*)obj)->ToPython();
			}
			return BoundMethodToPythonValue(value->ToMethod());
		}
		if (value->IsObject()) {
			BoundObject *obj = value->ToObject();
			std::cout << "type id of python object => " << typeid(obj).name() << std::endl;
			if (typeid(obj) == typeid(PythonValue*))
			{
				return ((PythonValue*)obj)->ToPython();
			}
			return BoundObjectToPythonValue(NULL,NULL,value->ToObject());
		}
		if (value->IsString()) {
			return PyString_FromString(value->ToString().c_str());
		}
		if (value->IsDouble()) {
			return PyFloat_FromDouble(value->ToDouble());
		}
		if (value->IsList()) {
	// 	return ValueListToPythonArray(value.ToArgList());
		}
		return NULL;
	}

	typedef struct {
	    PyObject_HEAD
		BoundObject *object;
	} PyBoundObject;

	static void PyBoundObject_dealloc(PyObject* self)
	{
		std::cout << "PyBoundObject_dealloc called" << std::endl;
		PyBoundObject *boundSelf = (PyBoundObject*)self;
		KR_DECREF(boundSelf->object);
		PyObject_Del(self);
	}

	PyObject* PyBoundObject_getattr(PyObject *self, char *name)
	{
		PyBoundObject *boundSelf = reinterpret_cast<PyBoundObject*>(self);
		std::cout << "PyBoundObject_getattr = 0x" << (void*)self << ", name=" << name << std::endl;
		std::cout << "PyBoundObject_getattr = 0x" << (void*)boundSelf->object << std::endl;
		Value* result = boundSelf->object->Get(name,NULL);
		std::cout << "after invoke" << std::endl;
		return ValueToPythonValue(result);
	}

	int PyBoundObject_setattr(PyObject *self, char *name, PyObject *value)
	{
		PyBoundObject *boundSelf = (PyBoundObject*)self;
		Value* tiValue = PythonValueToValue(value,name);
		boundSelf->object->Set(name,tiValue,NULL);
		return 0;
	}

	PyObject* PyBoundObject_tostring(PyObject *self)
	{
		PyBoundObject *boundSelf = (PyBoundObject*)self;
		Value* result = boundSelf->object->Get("toString",NULL);
		if (result->IsMethod())
		{
			BoundMethod *method = result->ToMethod();
			ValueList args;
			Value* toString = method->Call(args,NULL);
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
	    PyBoundObject_dealloc, 	/*tp_dealloc*/
	    0,          	 			/*tp_print*/
	    PyBoundObject_getattr,    /*tp_getattr*/
	    PyBoundObject_setattr,    /*tp_setattr*/
	    0,          	 			/*tp_compare*/
	    0,          	 			/*tp_repr*/
	    0,          	 			/*tp_as_number*/
	    0,          	 			/*tp_as_sequence*/
	    0,          	 			/*tp_as_mapping*/
	    0,           	 			/*tp_hash */
		0,							/*tp_call */
		PyBoundObject_tostring,	/*tp_str */
		0,							/*tp_getattro*/
		0,							/*tp_setattro*/
		0,							/*tp_as_buffer*/
		0,							/*tp_flags*/
		0,							/*tp_doc*/
	};

	PyObject* BoundObjectToPythonValue(PyObject* self, PyObject* args, BoundObject *bo)
	{
		//CHECK bo
		if (bo == NULL)
		{
			throw "BoundObject cannot be null";
		}
		PyBoundObject* obj;
		obj = PyObject_New(PyBoundObject, &PyBoundObjectType);
		obj->object = bo;
		KR_ADDREF(obj->object);
		return (PyObject*)obj;
	}

	void ThrowPythonException()
	{
		PyObject *ptype, *pvalue, *trace;
		PyErr_Fetch(&ptype,&pvalue,&trace);
		PyErr_Clear();
		throw PythonValueToValue(pvalue,NULL);
	}


	Value* PythonValueToValue(PyObject* value, const char *name)
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
		if (PyClass_Check(value))
		{
			PythonValue *v = new PythonValue(value);
			Value* tiv = new Value(v);
			return tiv;
		}
		if (PyInstance_Check(value))
		{
			PythonValue *v = new PythonValue(value);
			Value* tiv = new Value(v);
			return tiv;
		}
		if (PyMethod_Check(value))
		{
			BoundMethod *m = new PythonMethod(value,name);
			Value* tiv = new Value(m);
			return tiv;
		}
		if (PyFunction_Check(value))
		{
			BoundMethod *m = new PythonMethod(value,name);
			Value* tiv = new Value(m);
			return tiv;
		}
		if (PyCallable_Check(value))
		{
			BoundMethod *m = new PythonMethod(value,name);
			Value* tiv = new Value(m);
			return tiv;
		}
		if (PyObject_TypeCheck(value,&PyBoundObjectType))
		{
			std::cout << "python object is a PyBoundObjectType" << std::endl;
			PyBoundObject *o = reinterpret_cast<PyBoundObject*>(value);
			Value* tiv = new Value(o->object);
			return tiv;
		}

		std::cout << "PythonValueToValue:nothing" << std::endl;
		PyObject_Print(value,stdout,0);
		printf("\n");

		return new Value();
	}	
}


