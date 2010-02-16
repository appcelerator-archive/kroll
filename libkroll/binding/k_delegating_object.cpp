/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstring>

namespace kroll
{
	KDelegatingObject::KDelegatingObject(KObjectRef global) :
		global(global),
		local(new StaticBoundObject())
	{
	}

	KDelegatingObject::KDelegatingObject(
		KObjectRef global, KObjectRef local) :
		global(global),
		local(local)
	{
	}

	KDelegatingObject::~KDelegatingObject()
	{
	}

	KValueRef KDelegatingObject::Get(const char *name)
	{
		ScopedLock lock(&mutex);

		KValueRef val = local->Get(name);
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

	void KDelegatingObject::Set(const char *name, KValueRef value)
	{
		// We want to set the property on both
		// the local and the global object.
		ScopedLock lock(&mutex);
		local->Set(name, value);
		global->Set(name, value);
	}

	bool KDelegatingObject::HasProperty(const char* name)
	{
		return global->HasProperty(name) || local->HasProperty(name);
	}

	SharedStringList KDelegatingObject::GetPropertyNames()
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

