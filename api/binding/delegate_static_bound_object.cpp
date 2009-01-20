/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <cstring>

namespace kroll
{

	DelegateStaticBoundObject::DelegateStaticBoundObject(SharedBoundObject delegate)
		: StaticBoundObject(),
		  delegate(delegate)
	{
	}

	DelegateStaticBoundObject::~DelegateStaticBoundObject()
	{
		// The SharedPtr implementation should decrement
		// all members of properties, when the properties
		// map destructs
	}

	SharedValue DelegateStaticBoundObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		SharedValue val = StaticBoundObject::Get(name);
		if (!val->IsUndefined())
		{
			return val;
		}

		return this->delegate->Get(name);
	}

	void DelegateStaticBoundObject::Set(const char *name, SharedValue value)
	{
		ScopedLock lock(&mutex);
		StaticBoundObject::Set(name, value);
	}

	SharedStringList DelegateStaticBoundObject::GetPropertyNames()
	{
		ScopedLock lock(&mutex);

		SharedStringList delegate_list = delegate->GetPropertyNames();
		SharedStringList list = StaticBoundObject::GetPropertyNames();

		for (size_t i = 0; i < delegate_list->size(); i++)
		{
			bool found = false;
			for (size_t j = 0; j < list->size(); j++)
			{
				if (delegate_list->at(i).get() == list->at(j).get())
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list->push_back(delegate_list->at(i));
			}
		}

		return list;
	}

}

