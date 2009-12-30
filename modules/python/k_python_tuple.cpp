/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "k_python_list.h"

namespace kroll
{
	KPythonTuple::KPythonTuple(PyObject *tuple) :
		KList("Python.KPythonTuple"),
		tuple(tuple),
		object(new KPythonObject(tuple, true))
	{
		PyLockGIL lock;
		Py_INCREF(this->tuple);
	}

	KPythonTuple::~KPythonTuple()
	{
		PyLockGIL lock;
		Py_DECREF(this->tuple);
	}

	void KPythonTuple::Append(KValueRef value)
	{
		throw ValueException::FromString("Cannot modify the size of a Python tuple.");
	}

	unsigned int KPythonTuple::Size()
	{
		PyLockGIL lock;
		return PyTuple_Size(this->tuple);
	}

	bool KPythonTuple::Remove(unsigned int index)
	{
		throw ValueException::FromString("Cannot modify the size of a Python tuple.");
		return false;
	}

	KValueRef KPythonTuple::At(unsigned int index)
	{
		PyLockGIL lock;
		if (index >= 0 && index < this->Size())
		{
			PyObject *p = PyTuple_GetItem(this->tuple, index);
			KValueRef v = PythonUtils::ToKrollValue(p);
			Py_DECREF(p);
			return v;
		}
		else
		{
			return Value::Undefined;
		}
	}

	void KPythonTuple::Set(const char *name, KValueRef value)
	{
		throw ValueException::FromString("Cannot modify a Python tuple.");
	}

	void KPythonTuple::SetAt(unsigned int index, KValueRef value)
	{
		throw ValueException::FromString("Cannot modify a Python tuple.");
	}

	KValueRef KPythonTuple::Get(const char *name)
	{
		if (KList::IsInt(name))
		{
			return this->At(KList::ToIndex(name));
		}
		else
		{
			return object->Get(name);
		}
	}

	SharedStringList KPythonTuple::GetPropertyNames()
	{
		SharedStringList property_names = object->GetPropertyNames();
		for (size_t i = 0; i < this->Size(); i++)
		{
			std::string name = KList::IntToChars(i);
			property_names->push_back(new std::string(name));
		}
		return property_names;
	}

	PyObject* KPythonTuple::ToPython()
	{
		return this->object->ToPython();
	}

	bool KPythonTuple::Equals(KObjectRef other)
	{
		AutoPtr<KPythonTuple> pyOther = other.cast<KPythonTuple>();
		if (pyOther.isNull())
		{
			return false;
		}
		else
		{
			return pyOther->ToPython() == this->ToPython();
		}
	}
}
