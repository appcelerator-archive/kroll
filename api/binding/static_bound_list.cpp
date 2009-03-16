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
	StaticBoundList::StaticBoundList() :
		object(new StaticBoundObject()),
		length(0)
	{
	}

	StaticBoundList::~StaticBoundList()
	{
	}

	void StaticBoundList::Append(SharedValue value)
	{
		std::string name = KList::IntToChars(this->length);
		this->object->Set(name.c_str(), value);
		this->length++;
	}

	void StaticBoundList::SetAt(unsigned int index, SharedValue value)
	{
		std::string name = KList::IntToChars(index);
		this->object->Set(name.c_str(), value);
		if (index >= this->length)
			this->length = index + 1;
	}

	bool StaticBoundList::Remove(unsigned int index)
	{
		if (index < this->length)
		{
			std::string name = KList::IntToChars(index);
			this->object->UnSet(name.c_str());
			for (unsigned int i = index; i + 1 < this->length; i++)
				this->SetAt(i, this->At(i + 1));

			this->length--;
			return true;
		}
		else
		{
			return false;
		}
	}

	unsigned int StaticBoundList::Size()
	{
		return this->length;
	}

	SharedValue StaticBoundList::At(unsigned int index)
	{
		std::string name = KList::IntToChars(index);
		SharedValue value = this->object->Get(name.c_str());
		return value;
	}

	void StaticBoundList::Set(const char *name, SharedValue value)
	{
		int index = -1;
		if (KList::IsInt(name) && (index = atoi(name)) >= 0)
		{
			this->SetAt(index, value);
		}
		else
		{
			this->object->Set(name, value);
		}
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

