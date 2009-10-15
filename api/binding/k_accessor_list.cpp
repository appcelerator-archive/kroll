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
	KAccessorList::KAccessorList(const char* type)
		: StaticBoundList(type)
	{
	}

	bool KAccessorList::HasProperty(const char* name)
	{
		return StaticBoundList::HasProperty(name) ||
			!this->FindAccessorName(name).empty();
	}

	void KAccessorList::Set(const char* name, SharedValue value)
	{
		this->MapAccessor(name, value);

		std::string& accessorName = this->FindAccessorName(name);
		if (accessorName.empty())
		{
			StaticBoundList::Set(name, value);
			return;
		}

		SharedKMethod m(StaticBoundList::Get(accessorName.c_str())->ToMethod());
		if (m.isNull())
		{
			StaticBoundList::Set(name, value);
			return;
		}

		m->Call(value);
	}

	SharedValue KAccessorList::Get(const char* name)
	{
		std::string& accessorName = this->FindAccessorName(name);
		if (accessorName.empty())
			return StaticBoundList::Get(name);

		SharedKMethod m(StaticBoundList::Get(accessorName.c_str())->ToMethod());
		if (m.isNull())
			return StaticBoundList::Get(name);

		return m->Call();
	}
}

