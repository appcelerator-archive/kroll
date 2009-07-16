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

	static bool initializedPyKList = false;
	static bool initializedPyKMethod = false;
	static bool initializedPyKObject = false;

	typedef struct {
		PyObject_HEAD
		SharedValue* value;
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
		0,                          /*tp_doc*/
		0,                          /* tp_traverse */
		0,                          /* tp_clear */
		0,                          /* tp_richcompare */
		0,                          /* tp_weaklistoffset */
		0,                          /* tp_iter */
		0,                          /* tp_iternext */
		0,                          /* tp_methods */
		0,                          /* tp_members */
		0,                          /* tp_getset */
		0                           /* tp_base */
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
		0,                          /*tp_doc*/
		0,                          /* tp_traverse */
		0,                          /* tp_clear */
		0,                          /* tp_richcompare */
		0,                          /* tp_weaklistoffset */
		0,                          /* tp_iter */
		0,                          /* tp_iternext */
		0,                          /* tp_methods */
		0,                          /* tp_members */
		0,                          /* tp_getset */
		0                           /* tp_base */
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
		0,                          /*tp_doc*/
		0,                          /* tp_traverse */
		0,                          /* tp_clear */
		0,                          /* tp_richcompare */
		0,                          /* tp_weaklistoffset */
		0,                          /* tp_iter */
		0,                          /* tp_iternext */
		0,                          /* tp_methods */
		0,                          /* tp_members */
		0,                          /* tp_getset */
		0                           /* tp_base */
	};

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
		if (value->IsString())
		{
			return PyString_FromString(value->ToString());
		}
		if (value->IsMethod())
		{
			AutoPtr<KPythonMethod> pymeth = value->ToMethod().cast<KPythonMethod>();
			if (!pymeth.isNull())
			{
				return pymeth->ToPython();
			}
			else
			{
				return PythonUtils::KMethodToPyObject(value);
			}
		}
		if (value->IsList())
		{
			AutoPtr<KPythonList> pylist = value->ToList().cast<KPythonList>();
			if (!pylist.isNull())
			{
				return pylist->ToPython();
			}

			AutoPtr<KPythonTuple> pytuple = value->ToList().cast<KPythonTuple>();
			if (!pytuple.isNull())
			{
				return pytuple->ToPython();
			}

			return PythonUtils::KListToPyObject(value);
		}
		if (value->IsObject())
		{
			SharedKObject obj = value->ToObject();

			AutoPtr<KPythonObject> pyobj = obj.cast<KPythonObject>();
			if (!pyobj.isNull())
				return pyobj->ToPython();

			AutoPtr<KPythonDict> pydict = obj.cast<KPythonDict>();
			if (!pydict.isNull())
				return pydict->ToPython();

			return PythonUtils::KObjectToPyObject(value);
		}

		Py_INCREF(Py_None);
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

	SharedValue PythonUtils::ToKrollValue(PyObject* value)
	{
		if (Py_None == value)
		{
			return Value::Null;
		}
		else if (PyString_Check(value))
		{
			std::string s = PythonUtils::ToString(value);
			return Value::NewString(s);
		}
		else if (PyBool_Check(value))
		{
			return Value::NewBool(PyObject_IsTrue(value));
		}
		else if (PyInt_Check(value))
		{
			long lval = PyInt_AsLong(value);
			return Value::NewInt((int) lval);
		}
		else if (PyLong_Check(value))
		{
			double dval = PyLong_AsDouble(value);
			if (dval == -1.0)
			{
				THROW_PYTHON_EXCEPTION
			}
			return Value::NewDouble(dval);
		}
		else if (PyFloat_Check(value))
		{
			double dval = PyFloat_AsDouble(value);
			return Value::NewDouble(dval);
		}
		else if (PyList_Check(value))
		{
			SharedKList l = new KPythonList(value);
			SharedValue til = Value::NewList(l);
			return til;
		}
		else if (PyTuple_Check(value))
		{
			SharedKList l = new KPythonTuple(value);
			SharedValue til = Value::NewList(l);
			return til;
		}

		/* These are objects that originated in the binding layer.
		 * We need to unwrap them when we pass them back to Kroll */
		if (PyObject_TypeCheck(value, &PyKObjectType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			return *(o->value);
		}
		else if (PyObject_TypeCheck(value, &PyKMethodType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			return *(o->value);
		}
		else if (PyObject_TypeCheck(value, &PyKListType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			return *(o->value);
		}
		else if (PyInstance_Check(value))
		{
			SharedKObject v = new KPythonObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}
		else if (PyMethod_Check(value))
		{
			SharedKMethod m = new KPythonMethod(value);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		else if (PyModule_Check(value))
		{
			SharedKObject v = new KPythonObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}
		else if (PyFunction_Check(value))
		{
			SharedKMethod m = new KPythonMethod(value);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		else if (PyCallable_Check(value))
		{
			SharedKMethod m = new KPythonMethod(value);
			SharedValue tiv = Value::NewMethod(m);
			return tiv;
		}
		else if (PyMapping_Check(value))
		{
			// While dicts are read-only we bind mappable
			// objects as if gets/sets actually set the map keys
			SharedKObject o = new KPythonDict(value);
			SharedValue kv = Value::NewObject(o);
			return kv;
		}
		else
		{
			// This is likely a new-style object instance
			// and we can just map it like a KPythonObject
			SharedKObject v = new KPythonObject(value);
			SharedValue tiv = Value::NewObject(v);
			return tiv;
		}

	}

	static void PyKObject_dealloc(PyObject* self)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);

		Py_BEGIN_ALLOW_THREADS
		delete pyko->value;
		Py_END_ALLOW_THREADS

		PyObject_Del(self);
	}

	static PyObject* PyKObject_getattr(PyObject *self, char *name)
	{
		Py_INCREF(self);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);

		SharedValue result = 0;
		Py_BEGIN_ALLOW_THREADS
		result = pyko->value->get()->ToObject()->Get(name);
		Py_END_ALLOW_THREADS

		Py_DECREF(self);
		return PythonUtils::ToPyObject(result);
	}

	static int PyKObject_setattr(PyObject *self, char *name, PyObject *value)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);
		Py_INCREF(self);
		SharedValue tiValue = PythonUtils::ToKrollValue(value);

		Py_BEGIN_ALLOW_THREADS
		pyko->value->get()->ToObject()->Set(name, tiValue);
		Py_END_ALLOW_THREADS

		Py_DECREF(self);
		return 0;
	}

	static PyObject* PyKObject_str(PyObject *self)
	{
		Py_INCREF(self);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);
		SharedKObject kobj = pyko->value->get()->ToObject();
		Py_DECREF(self);

		SharedString ss = 0;
		Py_BEGIN_ALLOW_THREADS
		ss = kobj->DisplayString();
		Py_END_ALLOW_THREADS

		return PyString_FromString(ss->c_str());
	}

	PyObject* PythonUtils::KObjectToPyObject(SharedValue v)
	{
		if (!initializedPyKObject)
		{
			initializedPyKObject = true;
			if (PyType_Ready(&PyKObjectType) < 0)
				throw ValueException::FromString("Could not initialize PyKObjectType!");
		}

		PyKObject* obj = PyObject_New(PyKObject, &PyKObjectType);
		obj->value = new SharedValue(v);
		return (PyObject*) obj;
	}

	static Py_ssize_t PyKListLength(PyObject* o)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		SharedKList klist = pyko->value->get()->ToList();

		unsigned int size = 0;
		Py_BEGIN_ALLOW_THREADS
		size = klist->Size();
		Py_END_ALLOW_THREADS

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
		SharedKList klist = pyko->value->get()->ToList();

		SharedValue listVal = 0;
		Py_BEGIN_ALLOW_THREADS
		if (i < (int) klist->Size())
		{
			SharedValue listVal = klist->At(i);
		}
		Py_END_ALLOW_THREADS

		if (!listVal.isNull())
			return PythonUtils::ToPyObject(listVal);
		else
			return NULL;
	}

	static int PyKListSetItem(PyObject *o, Py_ssize_t i, PyObject *v)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		SharedKList klist = pyko->value->get()->ToList();
		SharedValue kv = PythonUtils::ToKrollValue(v);

		Py_BEGIN_ALLOW_THREADS
		klist->SetAt((unsigned int) i, kv);
		Py_END_ALLOW_THREADS

		return 1;
	}

	static int PyKListContains(PyObject *o, PyObject *value)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		SharedKList klist = pyko->value->get()->ToList();
		SharedValue kv = PythonUtils::ToKrollValue(value);

		Py_BEGIN_ALLOW_THREADS
		for (unsigned int i = 0; i < klist->Size(); i++)
		{
			if (kv == klist->At(i))
				return 1;
		}
		Py_END_ALLOW_THREADS

		return 0;
	}

	static PyObject* PyKListInPlaceConcat(PyObject *o1, PyObject *o2)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o1);
		SharedKList klist = pyko->value->get()->ToList();
		int size = PySequence_Size(o2);
		for (int i = 0; i < size; i++)
		{
			PyObject* v = PySequence_GetItem(o2, i);
			SharedValue kv = PythonUtils::ToKrollValue(v);

			Py_BEGIN_ALLOW_THREADS
			klist->Append(kv);
			Py_END_ALLOW_THREADS

		}
		return o1;
	}

	static PyObject* PyKListInPlaceRepeat(PyObject *o, Py_ssize_t count)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		SharedKList klist = pyko->value->get()->ToList();

		Py_BEGIN_ALLOW_THREADS
		unsigned int size = klist->Size();
		while (count > 0)
		{
			for (unsigned int i = 0; i < size; i++)
			{
				klist->Append(klist->At(i));
			}
			count--;
		}
		Py_END_ALLOW_THREADS

		return o;
	}

	PySequenceMethods KPySequenceMethods = {
		0
	};

	PyObject* PythonUtils::KListToPyObject(SharedValue v)
	{
		if (!initializedPyKList)
		{
			initializedPyKList = true;

			KPySequenceMethods.sq_length = &PyKListLength;
			KPySequenceMethods.sq_concat = &PyKListConcat;
			KPySequenceMethods.sq_repeat = &PyKListRepeat;
			KPySequenceMethods.sq_item = &PyKListGetItem;
			KPySequenceMethods.sq_ass_item = &PyKListSetItem;
			KPySequenceMethods.sq_inplace_concat = &PyKListInPlaceConcat;
			KPySequenceMethods.sq_contains = &PyKListContains;
			KPySequenceMethods.sq_inplace_repeat = &PyKListInPlaceRepeat;

			PyKListType.tp_as_sequence = &KPySequenceMethods;
			PyKListType.tp_flags = Py_TPFLAGS_HAVE_INPLACEOPS;

			if (PyType_Ready(&PyKListType) < 0)
				throw ValueException::FromString("Could not initialize PyKListType!");
		}

		PyKObject* obj = PyObject_New(PyKObject, &PyKListType);
		obj->value = new SharedValue(v);
		return (PyObject*) obj;

	}

	static PyObject* PyKMethod_call(PyObject *o, PyObject *args, PyObject *kw)
	{
		Py_INCREF(o);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		SharedKMethod kmeth = pyko->value->get()->ToMethod();

		ValueList a;
		SharedValue result = Value::Undefined;
		try
		{
			for (int c=0; c < PyTuple_Size(args); c++)
			{
				PyObject* arg = PyTuple_GetItem(args, c);
				SharedValue kValue = PythonUtils::ToKrollValue(arg);
				Value::Unwrap(kValue);
				a.push_back(kValue);
			}

			Py_BEGIN_ALLOW_THREADS
			result = kmeth->Call(a);
			Py_END_ALLOW_THREADS

		}
		catch (ValueException& e)
		{
			printf("threw exception on method\n");
			PyObject* pye = PythonUtils::ToPyObject(e.GetValue());
			PyObject* type = PyObject_Type(pye);
			PyErr_SetObject(type, pye);
			Py_DECREF(type);
			Py_DECREF(pye);
			Py_DECREF(o);
			return NULL;
		}

		Py_DECREF(o);
		return PythonUtils::ToPyObject(result);
	}

	PyObject* PythonUtils::KMethodToPyObject(SharedValue v)
	{
		if (!initializedPyKMethod)
		{
			initializedPyKMethod = true;
			if (PyType_Ready(&PyKMethodType) < 0)
				throw ValueException::FromString("Could not initialize PyKMethodType!");
		}

		PyKObject* obj = PyObject_New(PyKObject, &PyKMethodType);
		obj->value = new SharedValue(v);
		return (PyObject*) obj;
	}



}


