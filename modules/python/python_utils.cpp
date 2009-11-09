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
		KValueRef* value;
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
		0,                          /* tp_richcompare */ 0,                          /* tp_weaklistoffset */
		0,                          /* tp_iter */
		0,                          /* tp_iternext */
		0,                          /* tp_methods */
		0,                          /* tp_members */
		0,                          /* tp_getset */
		0                           /* tp_base */
	};

	PySequenceMethods KPySequenceMethods = { 0 };

	void PythonUtils::InitializePythonKClasses()
	{
		PyLockGIL lock;

		KPySequenceMethods.sq_length = &PyKListLength;
		KPySequenceMethods.sq_concat = &PyKListConcat;
		KPySequenceMethods.sq_repeat = &PyKListRepeat;
		KPySequenceMethods.sq_item = &PyKListGetItem;
		KPySequenceMethods.sq_ass_item = &PyKListSetItem;
		KPySequenceMethods.sq_contains = &PyKListContains;
		KPySequenceMethods.sq_inplace_concat = &PyKListInPlaceConcat;
		KPySequenceMethods.sq_inplace_repeat = &PyKListInPlaceRepeat;

		PyKListType.tp_as_sequence = &KPySequenceMethods;
		PyKListType.tp_flags = Py_TPFLAGS_HAVE_INPLACEOPS | Py_TPFLAGS_HAVE_SEQUENCE_IN;

		if (PyType_Ready(&PyKObjectType) < 0)
			throw ValueException::FromString("Could not initialize PyKObjectType!");

		if (PyType_Ready(&PyKListType) < 0)
			throw ValueException::FromString("Could not initialize PyKListType!");

		if (PyType_Ready(&PyKMethodType) < 0)
			throw ValueException::FromString("Could not initialize PyKMethodType!");

	}

	PyObject* PythonUtils::ToPyObject(KValueRef value)
	{
		PyLockGIL lock;
		PyObject* pythonValue = 0;
		bool needsReferenceIncrement = true;

		if (value->IsBool())
		{
			pythonValue = value->ToBool() ? Py_True : Py_False;
		}
		else if (value->IsDouble())
		{
			pythonValue = PyFloat_FromDouble(value->ToDouble());
		}
		else if (value->IsInt())
		{
			pythonValue = PyInt_FromLong(value->ToInt());
		}
		else if (value->IsNull() || value->IsUndefined())
		{
			pythonValue = Py_None;
		}
		else if (value->IsString())
		{
			pythonValue = PyString_FromString(value->ToString());
		}
		else if (value->IsMethod())
		{
			AutoPtr<KPythonMethod> pymeth = value->ToMethod().cast<KPythonMethod>();
			if (!pymeth.isNull())
			{
				pythonValue = pymeth->ToPython();
			}
			else
			{
				pythonValue = PythonUtils::KMethodToPyObject(value);
				needsReferenceIncrement = false;
			}
		}
		else if (value->IsList())
		{
			AutoPtr<KPythonList> pylist = value->ToList().cast<KPythonList>();
			AutoPtr<KPythonTuple> pytuple = value->ToList().cast<KPythonTuple>();
			if (!pylist.isNull())
			{
				pythonValue = pylist->ToPython();
			}
			else if (!pytuple.isNull())
			{
				pythonValue = pytuple->ToPython();
			}
			else
			{
				pythonValue = PythonUtils::KListToPyObject(value);
				needsReferenceIncrement = false;
			}
		}
		else if (value->IsObject())
		{
			KObjectRef obj = value->ToObject();
			AutoPtr<KPythonObject> pyobj = obj.cast<KPythonObject>();
			AutoPtr<KPythonDict> pydict = obj.cast<KPythonDict>();

			if (!pyobj.isNull())
			{
				pythonValue = pyobj->ToPython();
			}
			else if (!pydict.isNull())
			{
				pythonValue = pydict->ToPython();
			}
			else
			{
				pythonValue = PythonUtils::KObjectToPyObject(value);
				needsReferenceIncrement = false;
			}
		}

		if (!pythonValue)
			pythonValue = Py_None;

		if (needsReferenceIncrement)
			Py_INCREF(pythonValue);

		return pythonValue;
	}

	const char* PythonUtils::ToString(PyObject* value)
	{
		PyLockGIL lock;
		if (PyString_Check(value))
		{
			return PyString_AsString(value);
		}
		else
		{
			PyObject *o = PyObject_Str(value);
			const char* result = PyString_AsString(o);
			Py_DECREF(o);

			if (result)
				return result;
			else
				return "<python string failure>";
		}
	}

	KValueRef PythonUtils::ToKrollValue(PyObject* value)
	{
		PyLockGIL lock;

		// Snow Leopard's version of Python 2.6 seems to return false positives
		// when calling things like PyString_Check, PyList_Check, etc. Oddly enough,
		// calling PyObject_TypeCheck(...) with the appropriate arguments does the
		// trick. Therefore we should avoid using PyString_Check and friends for now
		// unless it does the right thing on Snow Leopard.

		KValueRef kvalue(0);
		if (Py_None == value)
		{
			kvalue = Value::Null;
		}
		else if (PyObject_TypeCheck(value, &PyString_Type))
		{
			kvalue = Value::NewString(PythonUtils::ToString(value));
		}
		else if (PyObject_TypeCheck(value, &PyUnicode_Type))
		{
			PyObject* utf8Value = PyUnicode_AsUTF8String(value);
			kvalue = Value::NewString(PythonUtils::ToString(utf8Value));
			Py_DECREF(utf8Value);
		}
		else if (PyObject_TypeCheck(value, &PyBool_Type))
		{
			kvalue = Value::NewBool(PyObject_IsTrue(value));
		}
		else if (PyObject_TypeCheck(value, &PyInt_Type))
		{
			kvalue = Value::NewInt((int) PyInt_AsLong(value));
		}
		else if (PyObject_TypeCheck(value, &PyLong_Type))
		{
			double dval = PyLong_AsDouble(value);
			if (dval == -1.0)
			{
				THROW_PYTHON_EXCEPTION
			}
			else
			{
				kvalue = Value::NewDouble(dval);
			}
		}
		else if (PyObject_TypeCheck(value, &PyFloat_Type))
		{
			kvalue = Value::NewDouble(PyFloat_AsDouble(value));
		}
		else if (PyObject_TypeCheck(value, &PyList_Type))
		{
			kvalue = Value::NewList(new KPythonList(value));
		}
		else if (PyObject_TypeCheck(value, &PyTuple_Type))
		{
			kvalue = Value::NewList(new KPythonTuple(value));
		}

		// These are objects that originated in the binding layer.
		// We need to unwrap them when we pass them back to Kroll.
		else if (PyObject_TypeCheck(value, &PyKObjectType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			kvalue = *(o->value);
		}
		else if (PyObject_TypeCheck(value, &PyKMethodType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			kvalue = *(o->value);
		}
		else if (PyObject_TypeCheck(value, &PyKListType))
		{
			PyKObject *o = reinterpret_cast<PyKObject*>(value);
			kvalue = *(o->value);
		}
		else if (PyObject_TypeCheck(value, &PyModule_Type))
		{
			kvalue = Value::NewObject(new KPythonObject(value));
		}
		else if (PyInstance_Check(value))
		{
			kvalue = Value::NewObject(new KPythonObject(value));
		}
		else if (PyMethod_Check(value))
		{
			kvalue = Value::NewMethod(new KPythonMethod(value));
		}
		else if (PyFunction_Check(value))
		{
			kvalue = Value::NewMethod(new KPythonMethod(value));
		}
		else if (PyCallable_Check(value))
		{
			kvalue = Value::NewMethod(new KPythonMethod(value));
		}
		else if (PyMapping_Check(value))
		{
			// While dicts are read-only we bind mappable
			// objects as if gets/sets actually set the map keys
			kvalue = Value::NewObject(new KPythonDict(value));
		}
		else
		{
			// This is likely a new-style object instance
			// and we can just map it like a KPythonObject
			kvalue = Value::NewObject(new KPythonObject(value));
		}

		if (kvalue.isNull())
		{
			std::string valueStr(PythonUtils::ToString(value));
			Logger::Get("Python.PythonUtils")->Error(
				"Failed to convert Python value to Kroll value: %s",
				valueStr.c_str());
			kvalue = Value::Undefined;
		}

		return kvalue;
	}

	static void PyKObject_dealloc(PyObject* self)
	{
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);

		{
			PyAllowThreads allow;
			delete pyko->value;
		}

		PyObject_Del(self);
	}

	static PyObject* PyKObject_getattr(PyObject *self, char *name)
	{
		PyLockGIL lock;
		Py_INCREF(self);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);

		KValueRef result = 0;
		{
			PyAllowThreads allow;
			result = pyko->value->get()->ToObject()->Get(name);
		}

		Py_DECREF(self);
		return PythonUtils::ToPyObject(result);
	}

	static int PyKObject_setattr(PyObject *self, char *name, PyObject *value)
	{
		PyLockGIL lock;
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);
		Py_INCREF(self);
		KValueRef tiValue = PythonUtils::ToKrollValue(value);

		{
			PyAllowThreads allow;
			pyko->value->get()->ToObject()->Set(name, tiValue);
		}

		Py_DECREF(self);
		return 0;
	}

	static PyObject* PyKObject_str(PyObject *self)
	{
		PyLockGIL lock;
		Py_INCREF(self);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(self);
		KObjectRef kobj = pyko->value->get()->ToObject();
		Py_DECREF(self);

		SharedString ss = 0;
		{
			PyAllowThreads allow;
			ss = kobj->DisplayString();
		}

		return PyString_FromString(ss->c_str());
	}

	PyObject* PythonUtils::KObjectToPyObject(KValueRef v)
	{
		PyLockGIL lock;
		PyKObject* obj = PyObject_New(PyKObject, &PyKObjectType);
		obj->value = new KValueRef(v);
		return (PyObject*) obj;
	}

	static Py_ssize_t PyKListLength(PyObject* o)
	{
		PyLockGIL lock;
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		KListRef klist = pyko->value->get()->ToList();

		unsigned int size = 0;
		{
			PyAllowThreads allow;
			size = klist->Size();
		}

		return (Py_ssize_t) size;
	}

	static PyObject* PyKListConcat(PyObject* a, PyObject* b)
	{
		PyLockGIL lock;
		PyObject* new_list = PyList_New(0);
		PySequence_Concat(new_list, a);
		PySequence_Concat(new_list, b);
		return new_list;
	}

	static PyObject* PyKListRepeat(PyObject *o, Py_ssize_t count)
	{
		PyLockGIL lock;
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
		PyLockGIL lock;
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		KListRef klist = pyko->value->get()->ToList();

		KValueRef listVal = 0;
		{
			PyAllowThreads allow;
			if (i < (int) klist->Size())
			{
				listVal = klist->At(i);
			}
		}

		if (!listVal.isNull())
			return PythonUtils::ToPyObject(listVal);
		else
			return NULL;
	}

	static int PyKListSetItem(PyObject *o, Py_ssize_t i, PyObject *v)
	{
		PyLockGIL lock;
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		KListRef klist = pyko->value->get()->ToList();
		KValueRef kv = PythonUtils::ToKrollValue(v);

		{
			PyAllowThreads allow;
			klist->SetAt((unsigned int) i, kv);
		}

		return 1;
	}

	static int PyKListContains(PyObject *o, PyObject *value)
	{
		PyLockGIL lock;
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		KListRef klist = pyko->value->get()->ToList();
		KValueRef kv = PythonUtils::ToKrollValue(value);

		{
			PyAllowThreads allow;
			for (unsigned int i = 0; i < klist->Size(); i++)
			{
				if (kv == klist->At(i))
					return 1;
			}
		}

		return 0;
	}

	static PyObject* PyKListInPlaceConcat(PyObject *o1, PyObject *o2)
	{
		PyLockGIL lock;
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o1);
		KListRef klist = pyko->value->get()->ToList();
		int size = PySequence_Size(o2);
		for (int i = 0; i < size; i++)
		{
			PyObject* v = PySequence_GetItem(o2, i);
			KValueRef kv = PythonUtils::ToKrollValue(v);

			{
				PyAllowThreads allow;
				klist->Append(kv);
			}

		}
		return o1;
	}

	static PyObject* PyKListInPlaceRepeat(PyObject *o, Py_ssize_t count)
	{
		PyLockGIL lock;
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		KListRef klist = pyko->value->get()->ToList();

		{
			PyAllowThreads allow;
			unsigned int size = klist->Size();
			while (count > 0)
			{
				for (unsigned int i = 0; i < size; i++)
				{
					klist->Append(klist->At(i));
				}
				count--;
			}
		}

		return o;
	}

	PyObject* PythonUtils::KListToPyObject(KValueRef v)
	{
		PyLockGIL lock;
		PyKObject* obj = PyObject_New(PyKObject, &PyKListType);
		obj->value = new KValueRef(v);
		return (PyObject*) obj;
	}

	static PyObject* PyKMethod_call(PyObject *o, PyObject *args, PyObject *kw)
	{
		PyLockGIL lock;
		Py_INCREF(o);
		PyKObject *pyko = reinterpret_cast<PyKObject*>(o);
		KMethodRef kmeth = pyko->value->get()->ToMethod();

		ValueList a;
		KValueRef result = Value::Undefined;
		try
		{
			for (int c = 0; c < PyTuple_Size(args); c++)
			{
				PyObject* arg = PyTuple_GetItem(args, c);
				KValueRef kValue = PythonUtils::ToKrollValue(arg);
				Value::Unwrap(kValue);
				a.push_back(kValue);
			}

			{
				PyAllowThreads allow;
				result = kmeth->Call(a);
			}

		}
		catch (ValueException& e)
		{
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

	PyObject* PythonUtils::KMethodToPyObject(KValueRef v)
	{
		PyLockGIL lock;
		PyKObject* obj = PyObject_New(PyKObject, &PyKMethodType);
		obj->value = new KValueRef(v);
		return (PyObject*) obj;
	}

	std::string PythonUtils::PythonErrorToString()
	{
		std::string fullTraceString("");
		PyObject *type, *value, *traceback;
		PyErr_Fetch(&type, &value, &traceback);

		PyObject* mod = PyImport_ImportModule("traceback");
		if (!mod)
		{
			return "";
		}
		else
		{
			char* fullTraceCString = NULL;

			PyObject* list = PyObject_CallMethod(mod,
				(char*) "format_exception", (char*) "OOO", type, value, traceback);

			if (list)
			{
				PyObject* newline = PyString_FromString("\n");
				PyObject* fullTrace = PyObject_CallMethod(newline,
					(char*) "join", (char*) "O", list);

				if (fullTrace)
					fullTraceCString = PyString_AsString(fullTrace);

				if (fullTraceCString)
					fullTraceString.append(fullTraceCString);

				Py_DECREF(list);
				Py_DECREF(newline);
				Py_DECREF(fullTrace);
			}

			// We failed to format the exception properly, so fall back
			// to using the str() of the exception.
			if (fullTraceString.empty())
				fullTraceString = PythonUtils::ToString(value);
		}
		return fullTraceString;
	}
}


