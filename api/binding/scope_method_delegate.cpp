/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "binding.h"

using namespace kroll;

ScopeMethodDelegate::ScopeMethodDelegate(MethodDelegateType type,
                                         SharedPtr<BoundObject> global,
                                         SharedPtr<BoundObject> scope,
                                         SharedPtr<BoundMethod> delegate) :
	type(type), global(global), scope(scope), delegate(delegate)
{
	//KR_ADDREF(global);
	//KR_ADDREF(scope);
	//KR_ADDREF(delegate);
}

ScopeMethodDelegate::~ScopeMethodDelegate()
{
	//KR_DECREF(global);
	//KR_DECREF(scope);
	//KR_DECREF(delegate);
}


void ScopeMethodDelegate::Set(const char *name, SharedPtr<Value> value)
{
	delegate->Set(name,value);
}

SharedPtr<Value> ScopeMethodDelegate::Get(const char *name)
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

SharedPtr<Value> ScopeMethodDelegate::Call(const ValueList& args)
{
	std::string key = args.at(0)->ToString();
	SharedPtr<BoundObject> obj = IsGlobalKey(key) ? global : scope;
	if (type == GET)
	{
		// not found, look inside scope
		return obj->Get(key.c_str());
	}
	else
	{
		obj->SetNS(key.c_str(),args.at(1));
		return Value::Undefined;
	}
}

SharedPtr<StaticBoundObject> ScopeMethodDelegate::CreateDelegate(SharedPtr<BoundObject> global, SharedPtr<BoundObject> bo)
{
	SharedPtr<StaticBoundObject> *scope = new StaticBoundObject();
	SharedStringList keys = bo->GetPropertyNames();
	SharedStringIter iter = keys.begin();

	while(iter!=keys.end())
	{
		const char *name = (*iter++);
		std::string key(name);
		SharedPtr<Value> value = bo->Get(name);

		if (key == "set")
		{
			SharedPtr<ScopeMethodDelegate> d = new ScopeMethodDelegate(SET, global, scope,value->ToMethod());
			SharedPtr<Value> v = new Value(d);
			//ScopedDereferencer d1(d);
			//ScopedDereferencer d2(v);
			scope->Set(name, v);
		}
		else if (key == "get")
		{
			SharedPtr<ScopeMethodDelegate> d = new ScopeMethodDelegate(GET, global, scope,value->ToMethod());
			SharedPtr<Value> v = new Value(d);
			//ScopedDereferencer d1(d);
			//ScopedDereferencer d2(v);
			scope->Set(name, v);
		}
		else
		{
			scope->Set(name, value);
		}

	}
	return scope;
}

