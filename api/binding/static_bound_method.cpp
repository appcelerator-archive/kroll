/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

namespace kroll
{
	StaticBoundMethod::StaticBoundMethod(MethodCallback* callback, const char *type)
		: KMethod(type), callback(callback)
	{
		this->object = new StaticBoundObject();
	}

	StaticBoundMethod::~StaticBoundMethod()
	{
	}

	KValueRef StaticBoundMethod::Call(const ValueList& args)
	{
		KValueRef tv = Value::NewUndefined();
		if (this->callback)
		{
			this->callback->Run(args, tv);
		}
		return tv;
	}

	void StaticBoundMethod::Set(const char *name, KValueRef value)
	{
		this->object->Set(name, value);
	}

	KValueRef StaticBoundMethod::Get(const char *name)
	{
		return this->object->Get(name);
	}

	SharedStringList StaticBoundMethod::GetPropertyNames()
	{
		return this->object->GetPropertyNames();
	}
}

