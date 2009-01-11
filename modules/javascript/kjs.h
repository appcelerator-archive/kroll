/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_H_
#define _KJS_H_

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>
#include <cstring>
#include <map>

#include <binding/binding.h>
#include "kjs_bound_object.h"
#include "kjs_bound_method.h"
#include "kjs_bound_list.h"
#include "javascriptapi.h"

namespace kroll
{
    KROLL_JAVASCRIPT_API void BindPropertyToJSObject(JSContextRef ctx,
	                            JSObjectRef o,
	                            const char * name,
	                            JSValueRef property);
    KROLL_JAVASCRIPT_API JSObjectRef KrollBoundObjectToJSValue(JSContextRef, kroll::BoundObject*);
	KROLL_JAVASCRIPT_API JSObjectRef KrollBoundMethodToJSValue(JSContextRef, kroll::BoundMethod*);
	KROLL_JAVASCRIPT_API JSObjectRef KrollBoundListToJSValue(JSContextRef, kroll::BoundList*);
	KROLL_JAVASCRIPT_API JSValueRef KrollValueToJSValue(JSContextRef, kroll::Value*);
	KROLL_JAVASCRIPT_API kroll::Value* JSValueToKrollValue(JSContextRef, JSValueRef, JSObjectRef);
	KROLL_JAVASCRIPT_API char* JSStringToChars(JSStringRef);

	// callbacks
	void  get_property_names_cb (JSContextRef, JSObjectRef, JSPropertyNameAccumulatorRef);
	bool has_property_cb (JSContextRef, JSObjectRef, JSStringRef);
	JSValueRef get_property_cb (JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);
	bool set_property_cb (JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);
	JSValueRef call_as_function_cb (JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	void finalize_cb(JSObjectRef);
}

#endif
