/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module.h"

namespace kroll
{

	static void PyKObject_dealloc(PyObject* );
	static PyObject* PyKObject_getattr(PyObject*, char*);
	static int PyKObject_setattr(PyObject*, char*, PyObject *);
	static PyObject* PyKObject_str(PyObject*);
	static Py_ssize_t PyKListLength(PyObject*);
	static PyObject* PyKListConcat(PyObject*, PyObject*);
	static PyObject* PyKListRepeat(PyObject*, Py_ssize_t);
	static PyObject* PyKListGetItem(PyObject*, Py_ssize_t);
	static int PyKListSetItem(PyObject*, Py_ssize_t, PyObject*);
	static int PyKListContains(PyObject*, PyObject*);
	static PyObject* PyKListInPlaceConcat(PyObject*, PyObject*);
	static PyObject* PyKListInPlaceRepeat(PyObject*, Py_ssize_t);
	static PyObject* PyKMethod_call(PyObject*, PyObject *, PyObject*);

	typedef struct {
		PyObject_HEAD
		SharedBoundObject* object;
	} PyKObject;

	static PyTypeObject PyKObjectType =
	{
		PyObject_HEAD_INIT(NULL)
		0,
		"KObject",
		sizeof(PyKObject),
		0,
		PyKObject_dealloc,          /*tp_dealloc*/
		0,                          /*tp_print*/
		PyKObject_getattr,          /*tp_getattr*/
		PyKObject_setattr,          /*tp_setattr*/
		0,                          /*tp_compare*/
		0,                          /*tp_repr*/
		0,                          /*tp_as_number*/
		0,                          /*tp_as_sequence*/
		0,                          /*tp_as_mapping*/
		0,                          /*tp_hash */
		0,                          /*tp_call */
		PyKObject_str,              /*tp_str */
		0,                          /*tp_getattro*/
		0,                          /*tp_setattro*/
		0,                          /*tp_as_buffer*/
		0,                          /*tp_flags*/
		0                           /*tp_doc*/
	};

	static PyTypeObject PyKMethodType =
	{
		PyObject_HEAD_INIT(NULL)
		0,
		"KMethod",
		sizeof(PyKObject),
		0,
		PyKObject_dealloc,          /*tp_dealloc*/
		0,                          /*tp_print*/
		PyKObject_getattr,          /*tp_getattr*/
		PyKObject_setattr,          /*tp_setattr*/
		0,                          /*tp_compare*/
		0,                          /*tp_repr*/
		0,                          /*tp_as_number*/
		0,                          /*tp_as_sequence*/
		0,                          /*tp_as_mapping*/
		0,                          /*tp_hash */
		PyKMethod_call,             /*tp_call */
		PyKObject_str,              /*tp_str */
		0,                          /*tp_getattro*/
		0,                          /*tp_setattro*/
		0,                          /*tp_as_buffer*/
		0,                          /*tp_flags*/
		0                           /*tp_doc*/
	};

	static PyTypeObject PyKListType =
	{
		PyObject_HEAD_INIT(NULL)
		0,
		"KList",
		sizeof(PyKObject),
		0,
		PyKObject_dealloc,          /*tp_dealloc*/
		0,                          /*tp_print*/
		PyKObject_getattr,          /*tp_getattr*/
		PyKObject_setattr,          /*tp_setattr*/
		0,                          /*tp_compare*/
		0,                          /*tp_repr*/
		0,                          /*tp_as_number*/
		0,                          /*tp_as_sequence*/
		0,                          /*tp_as_mapping*/
		0,                          /*tp_hash */
		0,                          /*tp_call */
		PyKObject_str,              /*tp_str */
		0,                          /*tp_getattro*/
		0,                          /*tp_setattro*/
		0,                          /*tp_as_buffer*/
		0,                          /*tp_flags*/
		0                           /*tp_doc*/
	};

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
			SharedBoundMethod meth = value->ToMethod();
			SharedPtr<KPythonMethod> pymeth = meth.cast<KPythonMethod>();
			if (!pymeth.isNull())
			{
				return pymeth->ToPython();
			}
			else
			{
				return PythonUtils::KMethodToPyObject(meth);
			}
		}
		if (value->IsList())
		{
			SharedBoundList list = value->ToList();
			SharedPtr<KPythonList> pylist = list.cast<KPythonList>();
			if (!pylist.isNull())
			{
				return pylist->ToPython();
			}
			else
			{
				return PythonUtils::KListToPyObject(list);
			}
		}
		if (value->IsObject())
		{
			SharedBoundObject obj = value->ToObject();
			SharedPtr<KPythonObject> pyobj = obj.cast<KPythonObject>();
			SharedPtr<KPythonDict> pydict = obj.cast<KPythonDict>();
			if (!pyobj.isNull())
			{
				return pyobj->ToPython();
			}
			if (!pydict.isNull())
			{
				return pydict->ToPython();
			}
			else
			{
				return PythonUtils::KObjectToPyObject(obj);
			}
		}
		if (value->IsString())
		{
			return PyString_FromString(value->ToString());
		}
		return Py_None;
	}

	const char* PythonUtils::ToString(PyObject* value)
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

	SharedValue PythonUtils::ToKrollValue(PyObject* value)
	{

		/* These are built-in Python types */
		if (Py_None==value)
		{
			return Value::Undefined;
		}
		else if (PyString_Check(value))
		{
			std::string s = PythonUtils::ToString(value);
			return Value::NewString(s);
		}
		else if (PyBool_Check(value))
		{
			return Value::NewBool(PythonBoolToBool(value));
		}
		else if (PyInt_Check(value))
		{
			return Value::NewInt(PythonFixnumToInt(value));
		}
		else if (PyFloat_Check(value))
		{
			return Value::NewDouble(PythonFloatToDouble(value));
		}
		else if (PyList_Check(value))
		{
			SharedBoundList l = new KPythonList(value);
			SharedValue til = Value::NewList(l);
			return til;
		}

		/* These are objects that originated in the binding layer.
		 * We need to unwrap them when we pass them back to Kroll */
		if (PyObject_TypeCheck(value, &PyKObjectType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			SharedValue tiv = Value::NewObject(*(o->object));
			return tiv;
		}
		else if (PyObject_TypeCheck(value, &PyKMethodType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			SharedBoundMethod meth = o->object->cast<BoundMethod>();
			SharedValue tiv = Value::NewMethod(meth);
			return tiv;
		}
		else if (PyObject_TypeCheck(value, &PyKListType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			SharedBoundList list = o->object->cast<BoundList>();
			SharedValue tiv = Value::NewList(list);
			return tiv;
		}
		else if (PyInstance_Check(value))
		{
			SharedBoundObject v = new KPythonObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}
		else if (PyMethod_Check(value))
		{
			SharedBoundMethod m = new KPythonMethod(value);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		else if (PyFunction_Check(value))
		{
			SharedBoundMethod m = new KPythonMethod(value);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		else if (PyCallable_Check(value))
		{
			SharedBoundMethod m = new KPythonMethod(value);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		else if (PyMapping_Check(value))
		{
			// While dicts are read-only we bind mappable
			// objects as if gets/sets actually set the map keys
			SharedBoundObject o = new KPythonDict(value);
			SharedValue kv = Value::NewObject(o);
			return kv;
		}
		else
		{
			// This is likely a new-style object instance
			// and we can just map it like a KPythonObject
			SharedBoundObject v = new KPythonObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}

	}

	static void PyKObject_dealloc(PyObject* self)
	{
		PyKObject *boundSelf = reinterpret_cast<PyKObject*>(self);
		delete boundSelf->object;
		PyObject_Del(self);
	}

	static PyObject* PyKObject_getattr(PyObject *self, char *name)
	{
		Py_INCREF(self);
		PyKObject *boundSelf = reinterpret_cast<PyKObject*>(self);
		SharedValue result = boundSelf->object->get()->Get(name);
		Py_DECREF(self);
		return PythonUtils::ToPyObject(result);
	}

	static int PyKObject_setattr(PyObject *self, char *name, PyObject *value)
	{
		PyKObject *boundSelf = reinterpret_cast<PyKObject*>(self);
		Py_INCREF(boundSelf);
		SharedValue tiValue = PythonUtils::ToKrollValue(value);
		boundSelf->object->get()->Set(name, tiValue);
		Py_DECREF(boundSelf);
		return 0;
	}

	static PyObject* PyKObject_str(PyObject *self)
	{
		Py_INCREF(self);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);
		SharedValue result = pyko->object->get()->Get("toString");
		Py_DECREF(self);

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
		sprintf(str,"<KObject %lx>", (unsigned long) pyko->object->get());
		return PyString_FromString(str);
	}

	PyObject* PythonUtils::KObjectToPyObject(SharedBoundObject bo)
	{
		PyKObject* obj = PyObject_New(PyKObject, &PyKObjectType);
		obj->object = new SharedBoundObject(bo);
		return (PyObject*)obj;
	}

	static Py_ssize_t PyKListLength(PyObject* o)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		BoundList* klist = reinterpret_cast<BoundList*>(pyko->object->get());
		unsigned int size = klist->Size();
		return (Py_ssize_t) size;
	}

	static PyObject* PyKListConcat(PyObject* a, PyObject* b)
	{
		PyObject* new_list = PyList_New(0);
		PySequence_Concat(new_list, a);
		PySequence_Concat(new_list, b);
		return new_list;
	}

	static PyObject* PyKListRepeat(PyObject *o, Py_ssize_t count)
	{
		PyObject* new_list = PyList_New(0);
		while (count > 0)
		{
			PySequence_Concat(new_list, o);
			count--;
		}
		return new_list;
	}

	static PyObject* PyKListGetItem(PyObject *o, Py_ssize_t i)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		BoundList* klist = reinterpret_cast<BoundList*>(pyko->object->get());
		if (i < (int) klist->Size())
		{
			return PythonUtils::ToPyObject(klist->At(i));
		}
		else
		{
			return NULL;
		}
	}

	static int PyKListSetItem(PyObject *o, Py_ssize_t i, PyObject *v)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		BoundList* klist = reinterpret_cast<BoundList*>(pyko->object->get());
		SharedValue kv = PythonUtils::ToKrollValue(v);
		std::string name = KList::IntToChars(i);
		klist->Set(name.c_str(), kv);
		return 1;
	}

	static int PyKListContains(PyObject *o, PyObject *value)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		BoundList* klist = reinterpret_cast<BoundList*>(pyko->object->get());
		SharedValue kv = PythonUtils::ToKrollValue(value);
		for (unsigned int i = 0; i < klist->Size(); i++)
		{
			if (kv == klist->At(i))
				return 1;
		}
		return 0;
	}

	static PyObject* PyKListInPlaceConcat(PyObject *o1, PyObject *o2)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o1);
		BoundList* klist = reinterpret_cast<BoundList*>(pyko->object->get());
		int size = PySequence_Size(o2);
		for (int i = 0; i < size; i++)
		{
			PyObject* v = PySequence_GetItem(o2, i);
			SharedValue kv = PythonUtils::ToKrollValue(v);
			klist->Append(kv);
		}
		return o1;
	}

	static PyObject* PyKListInPlaceRepeat(PyObject *o, Py_ssize_t count)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		BoundList* klist = reinterpret_cast<BoundList*>(pyko->object->get());
		unsigned int size = klist->Size();
		while (count > 0)
		{
			for (unsigned int i = 0; i < size; i++)
			{
				klist->Append(klist->At(i));
			}
			count--;
		}
		return o;
	}

	PySequenceMethods KPySequenceMethods = {
		0
	};

	PyObject* PythonUtils::KListToPyObject(SharedBoundObject bo)
	{
		if (KPySequenceMethods.sq_length == 0)
		{
			KPySequenceMethods.sq_length = &PyKListLength;
			KPySequenceMethods.sq_concat = &PyKListConcat;
			KPySequenceMethods.sq_repeat = &PyKListRepeat;
			KPySequenceMethods.sq_item = &PyKListGetItem;
			KPySequenceMethods.sq_ass_item = &PyKListSetItem;
			KPySequenceMethods.sq_inplace_concat = &PyKListInPlaceConcat;
			KPySequenceMethods.sq_contains = &PyKListContains;
			KPySequenceMethods.sq_inplace_repeat = &PyKListInPlaceRepeat;
		}
		if (PyKListType.tp_as_sequence == 0)
		{
			PyKListType.tp_as_sequence = &KPySequenceMethods;
			PyKListType.tp_flags = Py_TPFLAGS_HAVE_INPLACEOPS;
		}

		PyKObject* obj = PyObject_New(PyKObject, &PyKListType);
		obj->object = new SharedBoundObject(bo);
		return (PyObject*) obj;

	}

	static PyObject* PyKMethod_call(PyObject *o, PyObject *args, PyObject *kw)
	{
		Py_INCREF(o);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		BoundMethod* kmeth = reinterpret_cast<BoundMethod*>(pyko->object->get());

		ValueList a;
		SharedValue result = Value::Undefined;
		try
		{
			for (int c=0; c < PyTuple_Size(args); c++)
			{
				PyObject* arg = PyTuple_GetItem(args, c);
				a.push_back(PythonUtils::ToKrollValue(arg));
			}
			result = kmeth->Call(a);
		}
		catch (ValueException& e)
		{
			PyObject* pye = PythonUtils::ToPyObject(e.GetValue());
			PyObject* type = PyObject_Type(pye);
			PyErr_SetObject(type, pye);
			Py_DECREF(type);
			Py_DECREF(pye);
			return NULL;
		}

		Py_DECREF(o);
		return PythonUtils::ToPyObject(result);
	}

	PyObject* PythonUtils::KMethodToPyObject(SharedBoundObject bo)
	{
		PyKObject* obj = PyObject_New(PyKObject, &PyKMethodType);
		obj->object = new SharedBoundObject(bo);
		return (PyObject*) obj;
	}



}


