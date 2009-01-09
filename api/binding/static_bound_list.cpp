/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <cmath>
#include <cstdio>
#include <cstring>

namespace kroll
{
	StaticBoundList::StaticBoundList()
		: object(new StaticBoundObject())
	{
		this->Set("length", new Value(0), NULL);
	}

	StaticBoundList::~StaticBoundList()
	{
		KR_DECREF(this->object);
	}

	void StaticBoundList::Append(Value* value)
	{
		int length = this->Size();
		length = length + 1;
		char* name = StaticBoundList::IntToChars(length);
		this->object->Set(name, value, NULL);
		delete [] name;

		this->object->Set("length", new Value(length), NULL);
	}

	int StaticBoundList::Size()
	{
		Value *size_val = this->Get("length", NULL);
		if (size_val->IsInt())
		{
			return size_val->ToInt();
		}
		else
		{
			return 0;
		}
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
		if (StaticBoundList::IsInt(name))
		{
			int val = atoi(name);
			if (val > this->Size())
			{
				this->object->Set("length", new Value(val), NULL);
			}
		}

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

		// we've calculated the buffer length here, so we
		// sould be safe to use the usually unsafe sprintf
		// instead of platform-specific snprintf
		sprintf(str, "%d", value);
		return str;
	}

	bool StaticBoundList::IsInt(const char *name)
	{
		for (int i = 0; i < (int) strlen(name); i++)
		{
			if (!isdigit(name[i]))
				return false;
		}
		return true;
	}
}

