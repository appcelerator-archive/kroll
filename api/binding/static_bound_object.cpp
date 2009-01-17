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
		// all members of properties, when the properties
		// map destructs
	}

	SharedValue StaticBoundObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		std::map<std::string, SharedValue>::iterator iter;
		iter = properties.find(std::string(name));
		if (iter != properties.end()) {
			return iter->second;
		}

		return Value::Undefined;
	}

	void StaticBoundObject::Set(const char *name, SharedValue value)
	{
		ScopedLock lock(&mutex);

		this->UnSet(name);
		this->properties[std::string(name)] = value;
	}

	void StaticBoundObject::UnSet(const char *name)
	{
		ScopedLock lock(&mutex);

		std::map<std::string, SharedValue>::iterator iter;
		iter = this->properties.find(std::string(name));
		if (this->properties.end() != iter)
		{
			this->properties.erase(iter);
		}
	}

	SharedStringList StaticBoundObject::GetPropertyNames()
	{
		ScopedLock lock(&mutex);
		SharedStringList list(new StringList());

		std::map<std::string, SharedValue>::iterator iter;
		iter = properties.begin();
		while (iter != properties.end())
		{
			SharedString name_string(new std::string(iter->first));
			list->push_back(name_string);
			iter++;
		}

		return list;
	}

	void StaticBoundObject::SetObject(const char *name, SharedBoundObject object)
	{
		SharedValue obj_val = new Value(object);
		this->Set(name, obj_val);
	}
}

