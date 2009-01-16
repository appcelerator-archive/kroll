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
		SharedPtr<Value> len = new Value(0);
		this->Set("length", len);
		//KR_DECREF(len);
	}

	StaticBoundList::~StaticBoundList()
	{
		//KR_DECREF(this->object);
	}

	void StaticBoundList::Append(SharedPtr<Value> value)
	{
		int length = this->Size();
		char* name = StaticBoundList::IntToChars(length);
		this->object->Set(name, value);
		delete [] name;

		length = length + 1;
		SharedPtr<Value> len = new Value(length);
		this->object->Set("length", len);
		//KR_DECREF(len);
	}

	int StaticBoundList::Size()
	{
		SharedPtr<Value> size_val = this->Get("length");
		if (size_val->IsInt())
		{
			return size_val->ToInt();
		}
		else
		{
			return 0;
		}
	}

	SharedPtr<Value> StaticBoundList::At(int index)
	{
		char* name = StaticBoundList::IntToChars(index);
		SharedPtr<Value> value = this->object->Get(name);
		delete [] name;
		return value;
	}

	void StaticBoundList::Set(const char *name, SharedPtr<Value> value)
	{
		if (BoundList::IsNumber(name))
		{
			int val = atoi(name);
			if (val > this->Size())
			{
				SharedPtr<Value> len = new Value(val);
				this->object->Set("length", len);
				//KR_DECREF(len);
			}
		}

		this->object->Set(name, value);
	}

	SharedPtr<Value> StaticBoundList::Get(const char *name)
	{
		return this->object->Get(name);
	}

	SharedStringList StaticBoundList::GetPropertyNames()
	{
		return this->object->GetPropertyNames();
	}

	char* StaticBoundList::IntToChars(int value)
	{
		char buf[10];
		sprintf(buf,"%d",value);
		char *str = new char[strlen(buf)];
		strcpy(str,buf);
		return str;
	}

}

