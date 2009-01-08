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

	Value* StaticBoundMethod::Call(const ValueList& args, BoundObject* context)
	{
		Value* tv = new Value();
		this->callback->Run(args, tv, context);
		return tv;
	}
	void StaticBoundMethod::Set(const char *name, Value* value, BoundObject *context)
	{
		if (context!=NULL)
		{
			context->Set(name,value,NULL);
		}
		else
		{
			this->object->Set(name, value, context);
		}
	}

	Value* StaticBoundMethod::Get(const char *name, BoundObject *context)
	{
		if (context!=NULL)
		{
			return context->Get(name,NULL);
		}
		else
		{
			return this->object->Get(name, context);
		}
	}

	std::vector<std::string> StaticBoundMethod::GetPropertyNames()
	{
		return this->object->GetPropertyNames();
	}
}

