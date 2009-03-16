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
		static SharedValue ToKrollValue(JSValueRef, JSContextRef, JSObjectRef);
		static JSValueRef ToJSValue(SharedValue, JSContextRef);
		static JSValueRef KObjectToJSValue(SharedValue, JSContextRef);
		static JSValueRef KMethodToJSValue(SharedValue, JSContextRef);
		static JSValueRef KListToJSValue(SharedValue, JSContextRef);
		static char* ToChars(JSStringRef);
		static bool IsArrayLike(JSObjectRef, JSContextRef);
		static SharedPtr<KJSBoundObject> ToBoundObject(JSContextRef, JSObjectRef);
		static SharedPtr<KJSBoundMethod> ToBoundMethod(JSContextRef, JSObjectRef, JSObjectRef);
		static SharedPtr<KJSBoundList> ToBoundList(JSContextRef, JSObjectRef);
		static void RegisterGlobalContext(JSObjectRef, JSGlobalContextRef);
		static JSGlobalContextRef GetGlobalContext(JSObjectRef);

		static SharedValue Evaluate(JSContextRef context, char *script);
		static SharedValue EvaluateFile(JSContextRef context, char *full_path);

	};

}

#endif
