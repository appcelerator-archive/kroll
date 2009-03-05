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
		SharedValue len = Value::NewInt(0);
		this->Set("length", len);
	}

	StaticBoundList::~StaticBoundList()
	{
	}

	SharedBoundList StaticBoundList::FromStringVector(std::vector<std::string>& values)
	{
		SharedBoundList l = new StaticBoundList();
		std::vector<std::string>::iterator i = values.begin();
		while (i != values.end())
		{
			l->Append(Value::NewString(*i));
			i++;
		}
		return l;
	}

	bool StaticBoundList::Remove(unsigned int index)
	{
		unsigned int x = 0;
		bool found = false;
		for (unsigned int c = 0; c < this->Size(); c++)
		{
			if (c == index)
			{
				char* name = StaticBoundList::IntToChars(c);
				this->Set(name, Value::Undefined);
				delete [] name;
				found = true;
			}
			else
			{
				char* prop = StaticBoundList::IntToChars(x);
				this->Set(prop, this->At(c));
				delete [] prop;
				x++;
			}
		}
		this->object->Set("length", Value::NewInt(x));
		return found;
	}

	void StaticBoundList::Append(SharedValue value)
	{
		int length = this->Size();
		char* name = StaticBoundList::IntToChars(length);
		this->object->Set(name, value);
		delete [] name;

		SharedValue len = Value::NewInt(length+1);
		this->object->Set("length", len);
	}

	unsigned int StaticBoundList::Size()
	{
		SharedValue size_val = this->Get("length");
		if (size_val->IsInt())
		{
			return (unsigned int) size_val->ToInt();
		}
		else
		{
			return 0;
		}
	}

	SharedValue StaticBoundList::At(unsigned int index)
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
			if (val > (int) this->Size())
			{
				SharedValue len = Value::NewInt(val);
				this->object->Set("length", len);
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

	char* StaticBoundList::IntToChars(unsigned int value)
	{
		int digits = 1;
		if (value > 0)
			digits += floor(log10((double) value));

		char *buf = new char[digits + 1];
		sprintf(buf, "%d", value);
		return buf;
	}

}

