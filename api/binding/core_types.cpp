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
		/**
		 * @tiapi(method=True,name=CoreTypes.createKObject,since=0.4) create a kroll object
		 * @tiarg(for=CoreTypes.createKObject,name=object,type=any,optional=True) an optional object to wrap
		 * @tiresult(for=CoreTypes.createKObject,type=KObject) returns a kroll object
		 */
		this->SetMethod("createKObject", &CoreTypes::CreateKObject);
		
		/**
		 * @tiapi(method=True,name=CoreTypes.createKMethod,since=0.4) create a kroll method
		 * @tiarg(for=CoreTypes.createKMethod,name=function,type=function) a function object to wrap
		 * @tiresult(for=CoreTypes.createKMethod,type=KMethod) returns a kroll method
		 */
		this->SetMethod("createKMethod", &CoreTypes::CreateKMethod);
		
		/**
		 * @tiapi(method=True,name=CoreTypes.createKList,since=0.4) create a kroll list
		 * @tiarg(for=CoreTypes.createKList,name=list,type=Array<Any>,optional=True) an optional list to wrap
		 * @tiresult(for=CoreTypes.createKList,type=KObject) returns a kroll list
		 */
		this->SetMethod("createKList", &CoreTypes::CreateKList);
		
		/**
		 * @tiapi(method=True,name=CoreTypes.createBlob,since=0.4) create a blob (binary object)
		 * @tiarg(for=CoreTypes.createBlob,name=contents,type=string,optional=True) the blob contents
		 * @tiresult(for=CoreTypes.createBlob,type=Blob) returns a blob
		 */
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
