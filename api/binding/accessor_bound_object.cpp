/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"
#include <cstdio>
#include <cstring>

namespace kroll
{
	AccessorBoundObject::AccessorBoundObject()
		: StaticBoundObject()
	{
	}

	void AccessorBoundObject::Set(const char *name, SharedValue value)
	{
		std::string setter_name = "set" + Capitalize(name);
		SharedValue v = this->RawGet(setter_name.c_str());
		if (v->IsMethod())
		{
			SharedBoundMethod m = v->ToMethod();
			ValueList args;
			args.push_back(value);
			m->Call(args);
		}
		else
		{
			this->RawSet(name, value);
		}
	}

	SharedValue AccessorBoundObject::Get(const char *name)
	{
		std::string getter_name = "get" + Capitalize(name);
		SharedValue v = this->RawGet(getter_name.c_str());
		if (v->IsMethod())
		{
			SharedBoundMethod m = v->ToMethod();
			return m->Call(ValueList());
		}
		else
		{
			return this->RawGet(name);
		}
	}

	SharedValue AccessorBoundObject::RawGet(const char *name)
	{
		return StaticBoundObject::Get(name);
	}

	void AccessorBoundObject::RawSet(const char *name, SharedValue value)
	{
		StaticBoundObject::Set(name, value);
	}

	std::string AccessorBoundObject::Capitalize(const char *word)
	{
		char* cap_word = strdup(word);
		if (strlen(cap_word) > 0)
		{
			cap_word[0] = toupper(cap_word[0]);
		}

		std::string result = std::string(cap_word);
		free(cap_word);
		return result;
	}

}

