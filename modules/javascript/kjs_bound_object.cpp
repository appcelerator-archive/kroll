/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	KJSBoundObject::KJSBoundObject(JSContextRef context,
	                               JSObjectRef js_object)
		: context(context),
		  object(js_object)
	{
		JSValueProtect(context, js_object);
	}

	KJSBoundObject::~KJSBoundObject()
	{
		JSValueUnprotect(this->context, this->object);
	}

	JSObjectRef KJSBoundObject::GetJSObject()
	{
		return this->object;
	}

	SharedValue KJSBoundObject::Get(const char *name)
	{
		JSStringRef s = JSStringCreateWithUTF8CString(name);
		JSValueRef exception = NULL;
		JSValueRef js_value =
		    JSObjectGetProperty(this->context,
		                        this->object,
		                        s,
		                        NULL);
		JSStringRelease(s);

		if (exception != NULL) //exception thrown
		{
			SharedValue tv_exp = KJSUtil::ToKrollValue(exception, this->context, NULL);
			throw tv_exp;
		}

		return KJSUtil::ToKrollValue(js_value, this->context, this->object);
	}

	void KJSBoundObject::Set(const char *name, SharedValue value)
	{
		JSValueRef js_value = KJSUtil::ToJSValue(value, this->context);
		JSStringRef s = JSStringCreateWithUTF8CString(name);

		JSValueRef exception = NULL;
		JSObjectSetProperty(this->context,
		                    this->object,
		                    s,
		                    js_value,
		                    NULL, // attributes
		                    &exception);
		JSStringRelease(s);

		if (exception != NULL) //exception thrown
		{
			Value* tv_exp = KJSUtil::ToKrollValue(exception, this->context, NULL);
			throw tv_exp;
		}
	}

	SharedStringList KJSBoundObject::GetPropertyNames()
	{
		JSPropertyNameArrayRef names =
		                 JSObjectCopyPropertyNames(this->context, this->object);

		JSPropertyNameArrayRetain(names);

		SharedStringList list(new StringList());
		size_t count = JSPropertyNameArrayGetCount(names);
		for (size_t i = 0; i < count; i++)
		{
			JSStringRef js_name = JSPropertyNameArrayGetNameAtIndex(names, i);
			char* name = KJSUtil::ToChars(js_name);
			list->push_back(name);
		}

		JSPropertyNameArrayRelease(names);
		return list;
	}

	bool KJSBoundObject::SameContextGroup(JSContextRef c)
	{
		JSContextGroupRef context_group_a = JSContextGetGroup(this->context);
		JSContextGroupRef context_group_b = JSContextGetGroup(c);
		return context_group_a == context_group_b;
	}

}


