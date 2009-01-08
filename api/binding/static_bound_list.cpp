/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <cmath>
#include <stdio.h>

namespace kroll
{
	StaticBoundList::StaticBoundList()
		: object(new StaticBoundObject()),
		  length(0)
	{
	}

	StaticBoundList::~StaticBoundList()
	{
		KR_DECREF(this->object);
	}

	void StaticBoundList::Append(Value* value)
	{
		this->length++;
		char* name = StaticBoundList::IntToChars(this->length);
		this->object->Set(name, value);
		delete [] name;
	}

	int StaticBoundList::Size()
	{
		return this->length;
	}

	Value* StaticBoundList::At(int index)
	{
		char* name = StaticBoundList::IntToChars(index);
		Value *value = this->object->Get(name);
		delete [] name;
		return value;
	}

	void StaticBoundList::Set(const char *name, Value* value, BoundObject *context)
	{
		this->object->Set(name, value, context);
	}

	Value* StaticBoundList::Get(const char *name, BoundObject *context)
	{
		return this->object->Get(name, context);
	}

	std::vector<std::string> StaticBoundList::GetPropertyNames()
	{
		return this->object->GetPropertyNames();
	}

	char* StaticBoundList::IntToChars(int value)
	{
		int digits = (int) ceil(log((double)value));
		char* str = new char[digits + 1];
	#if defined(OS_WIN32)
		_snprintf(str, digits + 1, "%d", value);
	#else
		snprintf(str, digits + 1, "%d", value);
	#endif
		return str;
	}
}

