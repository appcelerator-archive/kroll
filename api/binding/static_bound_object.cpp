/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"

namespace kroll
{
	StaticBoundObject::StaticBoundObject()
	{
	}

	StaticBoundObject::~StaticBoundObject()
	{
		ScopedLock lock(&mutex);
		std::map<std::string, Value*>::iterator iter = properties.begin();
		while (iter != properties.end())
		{
			KR_DECREF(iter->second);
			iter++;
		}
	}

	Value* StaticBoundObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		std::map<std::string, Value*>::iterator iter;
		iter = properties.find(name);
		if (iter != properties.end()) {
			KR_ADDREF(iter->second);
			return iter->second;
		}

		return Value::Undefined();
	}

	void StaticBoundObject::Set(const char *name, Value* value)
	{
		ScopedLock lock(&mutex);

		KR_ADDREF(value);
		this->UnSet(name);
		this->properties[name] = value;
	}

	void StaticBoundObject::UnSet(const char *name)
	{
		ScopedLock lock(&mutex);
		std::map<std::string, Value*>::iterator iter = this->properties.find(name);
		if (this->properties.end() != iter)
		{
			KR_DECREF(iter->second);
			this->properties.erase(iter);
		}
	}

	void StaticBoundObject::GetPropertyNames(std::vector<const char *> *property_names)
	{
		ScopedLock lock(&mutex);
		std::map<std::string, Value*>::iterator iter = properties.begin();
		while (iter != properties.end())
		{
			property_names->push_back(strdup(iter->first.c_str()));
			iter++;
		}
	}

	void StaticBoundObject::SetObject(const char *name, BoundObject* object)
	{
		Value* obj_val = new Value(object);
		this->Set(name, obj_val);
		KR_DECREF(obj_val);
	}
}

kroll::RefCounted* CreateEmptyBoundObject()
{
	return new kroll::StaticBoundObject();
}
