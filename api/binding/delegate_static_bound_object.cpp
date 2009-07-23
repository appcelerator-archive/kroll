/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstring>

namespace kroll
{

	DelegateStaticBoundObject::DelegateStaticBoundObject(SharedKObject global) :
		global(global),
		local(new StaticBoundObject())
	{
	}

	DelegateStaticBoundObject::DelegateStaticBoundObject(
		SharedKObject global, SharedKObject local) :
		global(global),
		local(local)
	{
	}

	DelegateStaticBoundObject::~DelegateStaticBoundObject()
	{
	}

	SharedValue DelegateStaticBoundObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		SharedValue val = local->Get(name);
		if (!val->IsUndefined())
		{
			// We want properties of the local object to
			// override // properties set on the global object.
			return val;
		}
		else
		{
			// If the property isn't found on the local object, search
			// for it in the global object.
			return this->global->Get(name);
		}

	}

	void DelegateStaticBoundObject::Set(const char *name, SharedValue value)
	{
		// We want to set the property on both
		// the local and the global object.
		ScopedLock lock(&mutex);
		local->Set(name, value);
		global->Set(name, value);
	}

	bool DelegateStaticBoundObject::HasProperty(const char* name)
	{
		return global->HasProperty(name) || local->HasProperty(name);
	}

	SharedStringList DelegateStaticBoundObject::GetPropertyNames()
	{
		ScopedLock lock(&mutex);

		SharedStringList globalList = global->GetPropertyNames();
		SharedStringList localList = local->GetPropertyNames();

		for (size_t i = 0; i < globalList->size(); i++)
		{
			bool found = false;
			for (size_t j = 0; j < localList->size(); j++)
			{
				if (globalList->at(i).get() == localList->at(j).get())
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				localList->push_back(globalList->at(i));
			}
		}

		return localList;
	}

}

