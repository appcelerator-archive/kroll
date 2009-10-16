/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstring>

namespace kroll
{
	StaticBoundObject::StaticBoundObject(const char* type)
		: KObject(type)
	{
	}

	StaticBoundObject::~StaticBoundObject()
	{
		// The SharedPtr implementation should decrement
		// all members of properties, when the properties
		// map destructs
	}

	bool StaticBoundObject::HasProperty(const char *name)
	{
		return properties.find(name) != properties.end();
	}
	
	SharedValue StaticBoundObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		// TODO: Referencing global Undefined here is causing
		// invalid access in win32? (need to look into it..)
		SharedValue result = Value::NewUndefined();

		std::map<std::string, SharedValue>::iterator iter;
		iter = properties.find(std::string(name));
		if (iter != properties.end()) 
		{
			result = iter->second;
		}
		return result;
	}

	void StaticBoundObject::Set(const char *name, SharedValue value)
	{
		{
			ScopedLock lock(&mutex);
			this->UnSet(name);
			this->properties[std::string(name)] = value;
		}
		this->Bound(name,value);
	}

	void StaticBoundObject::UnSet(const char *name)
	{
		bool found = false;
		{
			ScopedLock lock(&mutex);
			std::map<std::string, SharedValue>::iterator iter;
			iter = this->properties.find(std::string(name));
			if (this->properties.end() != iter)
			{
				this->properties.erase(iter);
				found = true;
			}
		}
		
		if (found)
		{
			this->Unbound(name);
		}
	}

	SharedStringList StaticBoundObject::GetPropertyNames()
	{
		SharedStringList list(new StringList());

		ScopedLock lock(&mutex);
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

}
