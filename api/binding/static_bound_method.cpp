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
		KR_DECREF(this->object);
	}

	Value* StaticBoundMethod::Call(const ValueList& args)
	{
		Value* tv = new Value();
		this->callback->Run(args, tv);
		return tv;
	}
	void StaticBoundMethod::Set(const char *name, Value* value)
	{
		this->object->Set(name, value);
	}

	Value* StaticBoundMethod::Get(const char *name)
	{
		return this->object->Get(name);
	}

	void StaticBoundMethod::GetPropertyNames(std::vector<const char *> *property_names)
	{
		this->object->GetPropertyNames(property_names);
	}
}

