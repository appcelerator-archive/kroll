/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

namespace kroll
{

	ValueException::ValueException(SharedValue v) : std::exception(), value(v)
	{
	}

	ValueException::~ValueException() throw ()
	{
	}

	ValueException ValueException::FromString(const char* s)
	{
		return ValueException(Value::NewString(s));
	}

	ValueException ValueException::FromString(std::string s)
	{
		return ValueException(Value::NewString(s));
	}

	ValueException ValueException::FromObject(SharedKObject o)
	{
		return ValueException(Value::NewObject(o));
	}

	SharedValue ValueException::GetValue()
	{
		return this->value;
	}

	SharedString ValueException::DisplayString()
	{
		if (!this->value.isNull())
		{
			return this->value->DisplayString();
		}
		else
		{
			SharedString s = new std::string("<no exception>");
			return s;
		}
	}
}

