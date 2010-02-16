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
		return StaticBoundObject::HasProperty(name) || this->HasGetterFor(name);
	}

	void KAccessorObject::Set(const char* name, KValueRef value)
	{
		if (!this->UseSetter(name, value, StaticBoundObject::Get(name)))
			StaticBoundObject::Set(name, value);
	}

	KValueRef KAccessorObject::Get(const char* name)
	{
		return this->UseGetter(name, StaticBoundObject::Get(name));
	}
}

