/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
namespace kroll
{
	CoreTypes::CoreTypes()
	{
		this->SetMethod("createKObject", &CoreTypes::CreateKObject);
		this->SetMethod("createKMethod", &CoreTypes::CreateKMethod);
		this->SetMethod("createKList", &CoreTypes::CreateKList);
		this->SetMethod("createBlob", &CoreTypes::CreateBlob);
	}

	CoreTypes::~CoreTypes()
	{
	}

	void CoreTypes::CreateKObject(const ValueList& args, SharedValue result)
	{
		args.VerifyException("createKObject", "?o");
		if (args.size() <= 0)
		{
			result->SetObject(new StaticBoundObject());
		}
		else
		{
			SharedKObject wrapped = args.GetObject(0);
			result->SetObject(new KObjectWrapper(wrapped));
		}
	}

	void CoreTypes::CreateKMethod(const ValueList& args, SharedValue result)
	{
		args.VerifyException("createKMethod", "m");
		SharedKMethod wrapped = args.GetMethod(0);
		result->SetMethod(new KMethodWrapper(args.GetMethod(0)));
	}

	void CoreTypes::CreateKList(const ValueList& args, SharedValue result)
	{
		args.VerifyException("createKList", "?l");
		if (args.size() <= 0)
		{
			result->SetList(new StaticBoundList());
		}
		else
		{
			SharedKList wrapped = args.GetList(0);
			result->SetList(new KListWrapper(wrapped));
		}
	}

	void CoreTypes::CreateBlob(const ValueList& args, SharedValue result)
	{
		args.VerifyException("createBlob", "?s");
		if (args.size() > 0)
		{
			result->SetObject(new Blob(args.GetString(0)));
		}
		else
		{
			result->SetObject(new Blob());
		}
	}

	KObjectWrapper::KObjectWrapper(SharedKObject object) :
		object(object)
	{
	}

	void KObjectWrapper::Set(const char *name, SharedValue value)
	{
		object->Set(name, value);
	}

	SharedValue KObjectWrapper::Get(const char *name)
	{
		return object->Get(name);
	}

	SharedStringList KObjectWrapper::GetPropertyNames()
	{
		return object->GetPropertyNames();
	}

	SharedString KObjectWrapper::DisplayString(int levels)
	{
		return object->DisplayString(levels);
	}

	KMethodWrapper::KMethodWrapper(SharedKMethod method) :
		method(method)
	{
	}

	SharedValue KMethodWrapper::Call(const ValueList& args)
	{
		return method->Call(args);
	}

	void KMethodWrapper::Set(const char *name, SharedValue value)
	{
		method->Set(name, value);
	}

	SharedValue KMethodWrapper::Get(const char *name)
	{
		return method->Get(name);
	}

	SharedStringList KMethodWrapper::GetPropertyNames()
	{
		return method->GetPropertyNames();
	}

	SharedString KMethodWrapper::DisplayString(int levels)
	{
		return method->DisplayString(levels);
	}

	KListWrapper::KListWrapper(SharedKList list) :
		list(list)
	{
	}

	void KListWrapper::Append(SharedValue value)
	{
		list->Append(value);
	}

	unsigned int KListWrapper::Size()
	{
		return list->Size();
	}

	SharedValue KListWrapper::At(unsigned int index)
	{
		return list->At(index);
	}

	void KListWrapper::SetAt(unsigned int index, SharedValue value)
	{
		list->SetAt(index, value);
	}

	bool KListWrapper::Remove(unsigned int index)
	{
		return list->Remove(index);
	}

	void KListWrapper::Set(const char *name, SharedValue value)
	{
		list->Set(name, value);
	}

	SharedValue KListWrapper::Get(const char *name)
	{
		return list->Get(name);
	}

	SharedStringList KListWrapper::GetPropertyNames()
	{
		return list->GetPropertyNames();
	}

	SharedString KListWrapper::DisplayString(int levels)
	{
		return list->DisplayString(levels);
	}

}
