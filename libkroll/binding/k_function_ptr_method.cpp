/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

namespace kroll
{
	KFunctionPtrMethod::KFunctionPtrMethod(KFunctionPtrCallback callback) :
		callback(callback)
	{
		this->object = new StaticBoundObject();
	}

	KFunctionPtrMethod::~KFunctionPtrMethod()
	{
	}

	KValueRef KFunctionPtrMethod::Call(const ValueList& args)
	{
		return this->callback(args);
	}

	void KFunctionPtrMethod::Set(const char *name, KValueRef value)
	{
		this->object->Set(name, value);
	}

	KValueRef KFunctionPtrMethod::Get(const char *name)
	{
		return this->object->Get(name);
	}

	SharedStringList KFunctionPtrMethod::GetPropertyNames()
	{
		return this->object->GetPropertyNames();
	}
}

