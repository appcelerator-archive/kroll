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

	void StaticBoundList::Append(SharedValue value)
	{
		int length = this->Size();
		std::string name = KList::IntToChars(length);
		this->object->Set(name.c_str(), value);

		SharedValue len = Value::NewInt(length+1);
		this->object->Set("length", len);
	}

	void SetAt(unsigned int index, SharedValue value)
	{
		while (index >= this->Size())
		{
			// now we need to create entries between current size
			//  and new size and make the entries undefined.
			this->Append(Value::Undefined);
		}
		std::string name = KList::IntToChars(index);
		this->object->Set(name.c_str(), value);
	}

	bool StaticBoundList::Remove(unsigned int index)
	{
		unsigned int x = 0;
		bool found = false;
		for (unsigned int c = 0; c < this->Size(); c++)
		{
			if (c == index)
			{
				std::string name = KList::IntToChars(c);
				this->Set(name.c_str(), Value::Undefined);
				found = true;
			}
			else
			{
				std::string prop = KList::IntToChars(x);
				this->Set(prop.c_str(), this->At(c));
				x++;
			}
		}
		this->object->Set("length", Value::NewInt(x));
		return found;
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
		std::string name = KList::IntToChars(index);
		SharedValue value = this->object->Get(name.c_str());
		return value;
	}

	void StaticBoundList::Set(const char *name, SharedValue value)
	{
		if (KList::IsInt(name))
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

	SharedBoundList StaticBoundList::FromStringVector(std::vector<std::string>& values)
	{
		SharedKList l = new StaticBoundList();
		std::vector<std::string>::iterator i = values.begin();
		while (i != values.end())
		{
			l->Append(Value::NewString(*i));
			i++;
		}
		return l;
	}

}

