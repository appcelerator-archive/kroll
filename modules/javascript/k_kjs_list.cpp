/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"
#include <cstdio>

namespace kroll
{

	KKJSList::KKJSList(JSContextRef context, JSObjectRef js_object) :
		context(NULL),
		object(js_object)
	{
		/* KJS methods run in the global context that they originated from
		 * this seems to prevent nasty crashes from trying to access invalid
		 * contexts later. Global contexts need to be registered by all modules
		 * that use a KJS context. */
		JSObjectRef global_object = JSContextGetGlobalObject(context);
		JSGlobalContextRef global_context = KJSUtil::GetGlobalContext(global_object);

		// This context hasn't been registered. Something has gone pretty
		// terribly wrong and Kroll will likely crash soon. Nonetheless, keep
		// the user up-to-date to keep their hopes up.
		if (global_context == NULL)
			std::cerr << "Could not locate global context for a KJS method."  <<
			             " One of the modules is misbehaving." << std::endl;
		this->context = global_context;

		KJSUtil::ProtectGlobalContext(this->context);
		JSValueProtect(this->context, this->object);

		this->kjs_bound_object = new KKJSObject(this->context, this->object);
	}

	KKJSList::~KKJSList()
	{
		JSValueUnprotect(this->context, this->object);
		KJSUtil::UnprotectGlobalContext(this->context);
	}

	unsigned int KKJSList::Size()
	{
		SharedValue length_val = this->kjs_bound_object->Get("length");
		if (length_val->IsInt())
			return (unsigned int) length_val->ToInt();
		else
			return 0;
	}

	SharedValue KKJSList::At(unsigned int index)
	{
		std::string name = KList::IntToChars(index);
		SharedValue value = this->kjs_bound_object->Get(name.c_str());
		return value;
	}

	void KKJSList::SetAt(unsigned int index, SharedValue value)
	{
		std::string name = KList::IntToChars(index);
		this->kjs_bound_object->Set(name.c_str(), value);
	}

	void KKJSList::Append(SharedValue value)
	{
		SharedValue push_method = this->kjs_bound_object->Get("push");

		if (push_method->IsMethod())
		{
			ValueList list;
			list.push_back(value);
			push_method->ToMethod()->Call(list);
		}
		else
		{
			throw ValueException::FromString("Could not find push method on KJS array.");
		}
	}

	bool KKJSList::Remove(unsigned int index)
	{
		if (index >= 0 && index < this->Size())
		{
			SharedValue spliceMethod = this->kjs_bound_object->Get("splice");
			spliceMethod->ToMethod()->Call(
				Value::NewInt(index),
				Value::NewInt(1));
			return true;
		}
		return false;
	}


	SharedValue KKJSList::Get(const char *name)
	{
		return kjs_bound_object->Get(name);
	}

	void KKJSList::Set(const char *name, SharedValue value)
	{
		return kjs_bound_object->Set(name, value);
	}

	bool KKJSList::Equals(SharedKObject other)
	{
		SharedPtr<KKJSList> kjsOther = other.cast<KKJSList>();
		if (kjsOther.isNull())
			return false;
		if (!kjsOther->SameContextGroup(this->context))
			return false;
		return JSValueIsStrictEqual(
			this->context, this->object, kjsOther->GetJSObject());
	}

	SharedStringList KKJSList::GetPropertyNames()
	{
		 return kjs_bound_object->GetPropertyNames();
	}

	bool KKJSList::SameContextGroup(JSContextRef c)
	{
		return kjs_bound_object->SameContextGroup(c);
	}

	JSObjectRef KKJSList::GetJSObject()
	{
		return this->object;
	}



}
