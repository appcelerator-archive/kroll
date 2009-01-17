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
		SharedValue len = new Value(0);
		this->Set("length", len);
	}

	StaticBoundList::~StaticBoundList()
	{
	}

	void StaticBoundList::Append(SharedValue value)
	{
		int length = this->Size();
		char* name = StaticBoundList::IntToChars(length);
		this->object->Set(name, value);
		delete [] name;

		length = length + 1;
		SharedValue len = new Value(length);
		this->object->Set("length", len);
		//KR_DECREF(len);
	}

	int StaticBoundList::Size()
	{
		SharedValue size_val = this->Get("length");
		if (size_val->IsInt())
		{
			return size_val->ToInt();
		}
		else
		{
			return 0;
		}
	}

	SharedValue StaticBoundList::At(int index)
	{
		char* name = StaticBoundList::IntToChars(index);
		SharedValue value = this->object->Get(name);
		delete [] name;
		return value;
	}

	void StaticBoundList::Set(const char *name, SharedValue value)
	{
		if (BoundList::IsNumber(name))
		{
			int val = atoi(name);
			if (val > this->Size())
			{
				SharedValue len = new Value(val);
				this->object->Set("length", len);
				//KR_DECREF(len);
			}
		}

		this->object->Set(name, value);
	}

	SharedValue StaticBoundList::Get(const char *name)
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

