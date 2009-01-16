/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"

namespace kroll
{
	StaticBoundMethod::StaticBoundMethod(MethodCallback* callback)
		: callback(callback)
	{
		this->object = new StaticBoundObject();
	}

	StaticBoundMethod::~StaticBoundMethod()
	{
		//KR_DECREF(this->object);
	}

	SharedPtr<Value> StaticBoundMethod::Call(const ValueList& args)
	{
		SharedPtr<Value> tv = new Value();
		this->callback->Run(args, tv);
		return tv;
	}
	void StaticBoundMethod::Set(const char *name, SharedPtr<Value> value)
	{
		this->object->Set(name, value);
	}

	SharedPtr<Value> StaticBoundMethod::Get(const char *name)
	{
		return this->object->Get(name);
	}

	SharedStringList StaticBoundMethod::GetPropertyNames()
	{
		return this->object->GetPropertyNames();
	}
}

