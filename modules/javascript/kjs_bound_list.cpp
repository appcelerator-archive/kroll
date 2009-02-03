/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"
#include <cstdio>

namespace kroll
{

	KJSBoundList::KJSBoundList(JSContextRef context,
	                           JSObjectRef js_object)
		: context(context),
		  object(js_object)
	{
		JSValueProtect(context, js_object);
		this->kjs_bound_object = new KJSBoundObject(context, js_object);
	}

	KJSBoundList::~KJSBoundList()
	{
		JSValueUnprotect(this->context, this->object);
	}

	SharedValue KJSBoundList::Get(const char *name)
	{
		return kjs_bound_object->Get(name);
	}

	void KJSBoundList::Set(const char *name, SharedValue value)
	{
		return kjs_bound_object->Set(name, value);
	}

	SharedStringList KJSBoundList::GetPropertyNames()
	{
		 return kjs_bound_object->GetPropertyNames();
	}

	bool KJSBoundList::SameContextGroup(JSContextRef c)
	{
		return kjs_bound_object->SameContextGroup(c);
	}

	JSObjectRef KJSBoundList::GetJSObject()
	{
		return this->object;
	}

	void KJSBoundList::Append(SharedValue value)
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

	int KJSBoundList::Size()
	{
		SharedValue length_val = this->kjs_bound_object->Get("length");

		if (length_val->IsInt())
		{
			return length_val->ToInt();
		}
		else
		{
			return 0;
		}
	}

	SharedValue KJSBoundList::At(unsigned int index)
	{
		char* name = KJSBoundList::IntToChars(index);
		SharedValue value = this->kjs_bound_object->Get(name);
		delete [] name;
		return value;
	}

	char* KJSBoundList::IntToChars(int value)
	{
		int digits = 1;
		if (value > 0)
			digits += floor(log10((double) value));

		char *buf = new char[digits + 1];
		sprintf(buf, "%d", value);
		return buf;
	}

}
