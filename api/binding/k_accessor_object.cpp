/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstdio>
#include <cstring>

namespace kroll
{
	KAccessorObject::KAccessorObject(const char* name)
		: StaticBoundObject(name)
	{
	}

	bool KAccessorObject::HasProperty(const char* name)
	{
		return StaticBoundObject::HasProperty(name) ||
			!this->FindAccessorName(name).empty();
	}

	void KAccessorObject::Set(const char* name, SharedValue value)
	{
		this->MapAccessor(name, value);

		std::string& accessorName = this->FindAccessorName(name);
		if (accessorName.empty())
		{
			StaticBoundObject::Set(name, value);
			return;
		}

		SharedKMethod m(StaticBoundObject::Get(accessorName.c_str())->ToMethod());
		if (m.isNull())
		{
			StaticBoundObject::Set(name, value);
			return;
		}

		m->Call(value);
	}

	SharedValue KAccessorObject::Get(const char* name)
	{
		std::string& accessorName = this->FindAccessorName(name);
		if (accessorName.empty())
			return StaticBoundObject::Get(name);

		SharedKMethod m(StaticBoundObject::Get(accessorName.c_str())->ToMethod());
		if (m.isNull())
			return StaticBoundObject::Get(name);

		return m->Call();
	}
}

