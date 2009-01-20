/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"

using namespace kroll;

ScopeMethodDelegate::ScopeMethodDelegate(MethodDelegateType type,
                                         SharedBoundObject global,
                                         SharedBoundObject scope,
                                         SharedBoundMethod delegate) :
	type(type), global(global), scope(scope), delegate(delegate)
{
}

ScopeMethodDelegate::~ScopeMethodDelegate()
{
}


void ScopeMethodDelegate::Set(const char *name, SharedValue value)
{
	delegate->Set(name,value);
}

SharedValue ScopeMethodDelegate::Get(const char *name)
{
	return delegate->Get(name);
}

SharedStringList ScopeMethodDelegate::GetPropertyNames()
{
	return delegate->GetPropertyNames();
}

bool ScopeMethodDelegate::IsGlobalKey(std::string& key)
{
	std::string::size_type pos = key.find_first_of(".");
	return (pos!=std::string::npos);
}

SharedValue ScopeMethodDelegate::Call(const ValueList& args)
{
	std::string key = args.at(0)->ToString();
	SharedBoundObject obj = IsGlobalKey(key) ? global : scope;
	if (type == GET)
	{
		// not found, look inside scope
		return obj->Get(key.c_str());
	}
	else
	{
		SharedValue result = args.at(1);
		obj->SetNS(key.c_str(),result);
		return Value::Undefined;
	}
}

SharedPtr<StaticBoundObject> ScopeMethodDelegate::CreateDelegate(SharedBoundObject global, SharedBoundObject bo)
{
	SharedPtr<StaticBoundObject> scope = new StaticBoundObject();
	SharedStringList keys = bo->GetPropertyNames();
	StringList::iterator iter = keys->begin();

	while(iter!=keys->end())
	{
		SharedString key_ptr = (*iter++);
		std::string key = *key_ptr;
		SharedValue value = bo->Get(key.c_str());

		if (key == "set")
		{
			SharedBoundMethod d = new ScopeMethodDelegate(SET, global, scope, value->ToMethod());
			SharedValue v = Value::NewMethod(d);
			scope->Set(key.c_str(), v);
		}
		else if (key == "get")
		{
			SharedBoundMethod d = new ScopeMethodDelegate(GET, global, scope, value->ToMethod());
			SharedValue v = Value::NewMethod(d);
			scope->Set(key.c_str(), v);
		}
		else
		{
			scope->Set(key.c_str(), value);
		}

	}
	return scope;
}

