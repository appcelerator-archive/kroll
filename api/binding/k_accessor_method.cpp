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
	KAccessorMethod::KAccessorMethod(MethodCallback* callback, const char *type)
		: StaticBoundMethod(callback, type)
	{
	}

	bool KAccessorMethod::HasProperty(const char* name)
	{
		return StaticBoundMethod::HasProperty(name) ||
			!this->FindAccessorName(name).empty();
	}

	void KAccessorMethod::Set(const char* name, SharedValue value)
	{
		this->MapAccessor(name, value);

		std::string& accessorName = this->FindAccessorName(name);
		if (accessorName.empty())
		{
			StaticBoundMethod::Set(name, value);
			return;
		}

		SharedKMethod m(StaticBoundMethod::Get(accessorName.c_str())->ToMethod());
		if (m.isNull())
		{
			StaticBoundMethod::Set(name, value);
			return;
		}

		m->Call(value);
	}

	SharedValue KAccessorMethod::Get(const char* name)
	{
		std::string& accessorName = this->FindAccessorName(name);
		if (accessorName.empty())
			return StaticBoundMethod::Get(name);

		SharedKMethod m(StaticBoundMethod::Get(accessorName.c_str())->ToMethod());
		if (m.isNull())
			return StaticBoundMethod::Get(name);

		return m->Call();
	}
}

