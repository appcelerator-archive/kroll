/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstdio>
#include <cstring>
#include <Poco/Stopwatch.h>

namespace kroll
{
	ProfiledBoundMethod::ProfiledBoundMethod(KMethodRef delegate, std::string& type) :
		ProfiledBoundObject(delegate),
		method(delegate),
		fullType(type),
		count(1)
	{
	}

	ProfiledBoundMethod::~ProfiledBoundMethod()
	{
	}

	KValueRef ProfiledBoundMethod::Call(const ValueList& args)
	{
		std::string type = this->GetType();

		KValueRef value;
		Poco::Stopwatch sw;
		sw.start();
		try {
			value = method->Call(args);
		} catch (...) {
			sw.stop();
			this->Log("call", type, sw.elapsed());
			throw;
		}

		sw.stop();
		this->Log("call", type, sw.elapsed());
		return this->Wrap(value, type);
	}

	void ProfiledBoundMethod::Set(const char *name, KValueRef value)
	{
		method->Set(name,value);
	}

	KValueRef ProfiledBoundMethod::Get(const char *name)
	{
		return method->Get(name);
	}

	SharedStringList ProfiledBoundMethod::GetPropertyNames()
	{
		return method->GetPropertyNames();
	}

	bool ProfiledBoundMethod::HasProperty(const char* name)
	{
		return method->HasProperty(name);
	}

	std::string& ProfiledBoundMethod::GetType()
	{
		return fullType;
	}
}
