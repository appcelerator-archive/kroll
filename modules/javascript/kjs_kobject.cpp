/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	KJSKObject::KJSKObject(JSContextRef context, JSObjectRef js_object) :
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
	}

	KJSKObject::~KJSKObject()
	{
		JSValueUnprotect(this->context, this->object);
		KJSUtil::UnprotectGlobalContext(this->context);
	}

	JSObjectRef KJSKObject::GetJSObject()
	{
		return this->object;
	}

	SharedValue KJSKObject::Get(const char *name)
	{
		JSStringRef s = JSStringCreateWithUTF8CString(name);
		JSValueRef exception = NULL;
		JSValueRef js_value =
		    JSObjectGetProperty(this->context, this->object, s, NULL);
		JSStringRelease(s);

		if (exception != NULL) //exception thrown
		{
			SharedValue tv_exp = KJSUtil::ToKrollValue(exception, this->context, NULL);
			throw ValueException(tv_exp);
		}

		SharedValue kvalue = KJSUtil::ToKrollValue(js_value, this->context, this->object);
		return kvalue;
	}

	void KJSKObject::Set(const char *name, SharedValue value)
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
			throw ValueException(tv_exp);
		}
	}

	SharedStringList KJSKObject::GetPropertyNames()
	{
		SharedStringList list(new StringList());

		JSPropertyNameArrayRef names =
		                 JSObjectCopyPropertyNames(this->context, this->object);
		JSPropertyNameArrayRetain(names);

		size_t count = JSPropertyNameArrayGetCount(names);
		for (size_t i = 0; i < count; i++)
		{
			JSStringRef js_name = JSPropertyNameArrayGetNameAtIndex(names, i);
			char* name = KJSUtil::ToChars(js_name);
			SharedString name_str(new std::string(name));
			list->push_back(name_str);
		}

		JSPropertyNameArrayRelease(names);
		return list;
	}

	bool KJSKObject::SameContextGroup(JSContextRef c)
	{
		JSContextGroupRef context_group_a = JSContextGetGroup(this->context);
		JSContextGroupRef context_group_b = JSContextGetGroup(c);
		return context_group_a == context_group_b;
	}

}


