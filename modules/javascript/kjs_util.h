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
		static SharedValue ToKrollValue(JSValueRef value,
		                           JSContextRef context,
		                           JSObjectRef thisObject);

		static JSValueRef ToJSValue(SharedValue value,
		                            JSContextRef context);

		static JSValueRef ToJSValue(SharedBoundObject object,
		                            JSContextRef context);

		static JSValueRef ToJSValue(SharedBoundMethod method,
		                            JSContextRef context);

		static JSValueRef ToJSValue(SharedBoundList list,
		                            JSContextRef context);

		static char* ToChars(JSStringRef js_string);

		static bool IsArrayLike(JSObjectRef object,
		                        JSContextRef context);

		static SharedPtr<KJSBoundObject> ToBoundObject(JSContextRef context,
		                                               JSObjectRef object);

		static SharedPtr<KJSBoundMethod> ToBoundMethod(JSContextRef context,
		                                               JSObjectRef method,
		                                               JSObjectRef this_object);

		static SharedPtr<KJSBoundList> ToBoundList(JSContextRef context,
		                                           JSObjectRef list);

		static void RegisterGlobalContext(JSContextGroupRef,
		                                  JSGlobalContextRef);
		static JSGlobalContextRef GetGlobalContext(JSContextGroupRef);

	};

}

#endif
