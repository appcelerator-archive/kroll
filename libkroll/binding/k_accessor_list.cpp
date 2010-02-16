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
		return StaticBoundList::HasProperty(name) || this->HasGetterFor(name);
	}

	void KAccessorList::Set(const char* name, KValueRef value)
	{
		if (!this->UseSetter(name, value, StaticBoundList::Get(name)))
			StaticBoundList::Set(name, value);
	}

	KValueRef KAccessorList::Get(const char* name)
	{
		return this->UseGetter(name, StaticBoundList::Get(name));
	}
}

