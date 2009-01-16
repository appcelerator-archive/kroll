/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <cstring>

namespace kroll
{

	SharedBoundObject BoundObject::CreateEmptyBoundObject()
	{
		return new kroll::StaticBoundObject();
	}

	StaticBoundObject::StaticBoundObject()
	{
	}

	StaticBoundObject::~StaticBoundObject()
	{
		// The SharedPtr implementation should decrement
		// all members of properties, when the destrutor
		// is called on it
	}

	SharedValue StaticBoundObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		std::map<std::string, SharedValue>::iterator iter;
		iter = properties.find(name);
		if (iter != properties.end()) {
			//KR_ADDREF(iter->second);
			return iter->second;
		}

		return Value::Undefined;
	}

	void StaticBoundObject::Set(const char *name, SharedValue value)
	{
		ScopedLock lock(&mutex);

		//KR_ADDREF(value);
		this->UnSet(name);
		this->properties[name] = value;
	}

	void StaticBoundObject::UnSet(const char *name)
	{
		ScopedLock lock(&mutex);
		std::map<std::string, SharedValue>::iterator iter = this->properties.find(name);
		if (this->properties.end() != iter)
		{
			//KR_DECREF(iter->second);
			this->properties.erase(iter);
		}
	}

	SharedStringList StaticBoundObject::GetPropertyNames()
	{
		ScopedLock lock(&mutex);
		std::map<std::string, SharedValue>::iterator iter = properties.begin();
		SharedStringList list(new StringList());
		while (iter != properties.end())
		{
			list->push_back(iter->first.c_str());
			iter++;
		}

		return list;
	}

	void StaticBoundObject::SetObject(const char *name, SharedBoundObject object)
	{
		SharedValue obj_val = new Value(object);
		this->Set(name, obj_val);
		//KR_DECREF(obj_val);
	}
}

