/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	KJSBoundMethod::KJSBoundMethod(JSContextRef context,
	                               JSObjectRef js_object,
	                               JSObjectRef this_obj)
		: context(context),
		  object(js_object),
		  this_obj(this_obj)
	{
		JSValueProtect(context, js_object);

		if (this_obj != NULL)
			JSValueProtect(context, this_obj);

		this->kjs_bound_object = new KJSBoundObject(context, js_object);
	}

	KJSBoundMethod::~KJSBoundMethod()
	{
		JSValueUnprotect(this->context, this->object);

		if (this->this_obj != NULL)
			JSValueUnprotect(this->context, this->this_obj);

		//KR_DECREF(kjs_bound_object);
	}

	SharedValue KJSBoundMethod::Get(const char *name)
	{
		return kjs_bound_object->Get(name);
	}

	void KJSBoundMethod::Set(const char *name, SharedValue value)
	{
		return kjs_bound_object->Set(name, value);
	}

	SharedStringList KJSBoundMethod::GetPropertyNames()
	{
		return kjs_bound_object->GetPropertyNames();
	}

	bool KJSBoundMethod::SameContextGroup(JSContextRef c)
	{
		return kjs_bound_object->SameContextGroup(c);
	}

	JSObjectRef KJSBoundMethod::GetJSObject()
	{
		return this->object;
	}

	SharedValue KJSBoundMethod::Call(const ValueList& args)
	{

		JSValueRef* js_args = new JSValueRef[args.size()];
		for (int i = 0; i < (int) args.size(); i++)
		{
			SharedValue arg = args.at(i);
			js_args[i] = KJSUtil::ToJSValue(arg, this->context);
		}

		JSValueRef exception = NULL;
		JSValueRef js_value = JSObjectCallAsFunction(this->context,
		                                             this->object,
		                                             this->this_obj,
		                                             args.size(),
		                                             js_args,
		                                             &exception);
		delete [] js_args; // clean up args

		if (js_value == NULL && exception != NULL) //exception thrown
		{
			SharedValue tv_exp = KJSUtil::ToKrollValue(exception, this->context, NULL);
			throw tv_exp.get();
		}

		return KJSUtil::ToKrollValue(js_value, this->context, NULL);
	}

}

