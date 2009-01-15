/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <cstring>

namespace kroll
{

	SharedPtr<BoundObject> BoundObject::CreateEmptyBoundObject()
	{
		return new kroll::StaticBoundObject();
	}

	StaticBoundObject::StaticBoundObject()
	{
	}

	StaticBoundObject::~StaticBoundObject()
	{
		/*
		ScopedLock lock(&mutex);
		std::map<std::string, SharedPtr<Value> >::iterator iter = properties.begin();
		while (iter != properties.end())
		{
			KR_DECREF(iter->second);
			iter++;
		}*/
	}

	SharedPtr<Value> StaticBoundObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		std::map<std::string, SharedPtr<Value> >::iterator iter;
		iter = properties.find(name);
		if (iter != properties.end()) {
			//KR_ADDREF(iter->second);
			return iter->second;
		}

		return Value::Undefined;
	}

	void StaticBoundObject::Set(const char *name, SharedPtr<Value> value)
	{
		ScopedLock lock(&mutex);

		//KR_ADDREF(value);
		this->UnSet(name);
		this->properties[name] = value;
	}

	void StaticBoundObject::UnSet(const char *name)
	{
		ScopedLock lock(&mutex);
		std::map<std::string, SharedPtr<Value> >::iterator iter = this->properties.find(name);
		if (this->properties.end() != iter)
		{
			//KR_DECREF(iter->second);
			this->properties.erase(iter);
		}
	}

	SharedStringList StaticBoundObject::GetPropertyNames()
	{
		ScopedLock lock(&mutex);
		std::map<std::string, SharedPtr<Value> >::iterator iter = properties.begin();
		SharedStringList list(new StringList());
		while (iter != properties.end())
		{
			list->push_back(iter->first.c_str());
			iter++;
		}
	}

	void StaticBoundObject::SetObject(const char *name, SharedPtr<BoundObject> object)
	{
		SharedPtr<Value> obj_val = new Value(object);
		this->Set(name, obj_val);
		//KR_DECREF(obj_val);
	}
}

