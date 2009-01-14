/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_UTIL_H_
#define _KJS_UTIL_H_

namespace kroll
{

	class KROLL_JAVASCRIPT_API KJSUtil
	{

	public:
		static Value* ToKrollValue(JSValueRef value,
		                           JSContextRef context,
		                           JSObjectRef thisObject);

		static JSValueRef ToJSValue(Value* value,
		                            JSContextRef context);

		static JSValueRef ToJSValue(BoundObject *object,
		                            JSContextRef context);

		static JSValueRef ToJSValue(BoundMethod *method,
		                            JSContextRef context);

		static JSObjectRef ToJSValue(BoundList *list,
		                            JSContextRef context);

		static char* ToChars(JSStringRef js_string);

		static bool IsArrayLike(JSObjectRef object,
		                        JSContextRef context);

		static void BindPropertyToJSObject(JSContextRef context,
		                                   JSObjectRef object,
		                                   const char* name,
		                                   JSValueRef property);

	};

}

#endif
