/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_UTIL_H_
#define _KJS_UTIL_H_

namespace kroll
{
	namespace KJSUtil
	{
		KROLL_JAVASCRIPT_API KValueRef ToKrollValue(JSValueRef, JSContextRef, JSObjectRef);
		KROLL_JAVASCRIPT_API JSValueRef ToJSValue(KValueRef, JSContextRef);
		KROLL_JAVASCRIPT_API JSValueRef KObjectToJSValue(KValueRef, JSContextRef);
		KROLL_JAVASCRIPT_API JSValueRef KMethodToJSValue(KValueRef, JSContextRef);
		KROLL_JAVASCRIPT_API JSValueRef KListToJSValue(KValueRef, JSContextRef);
		KROLL_JAVASCRIPT_API std::string ToChars(JSStringRef);
		KROLL_JAVASCRIPT_API bool IsArrayLike(JSObjectRef, JSContextRef);
		KROLL_JAVASCRIPT_API AutoPtr<KKJSObject> ToBoundObject(JSContextRef, JSObjectRef);
		KROLL_JAVASCRIPT_API AutoPtr<KKJSObject> ToBoundMethod(JSContextRef, JSObjectRef, JSObjectRef);
		KROLL_JAVASCRIPT_API AutoPtr<KKJSList> ToBoundList(JSContextRef, JSObjectRef);

		KROLL_JAVASCRIPT_API JSObjectRef CreateNewGlobalContext(Host*, bool addGlobalObject=true);
		KROLL_JAVASCRIPT_API void RegisterGlobalContext(JSObjectRef, JSGlobalContextRef);
		KROLL_JAVASCRIPT_API void UnregisterGlobalContext(JSObjectRef);
		KROLL_JAVASCRIPT_API JSGlobalContextRef GetGlobalContext(JSObjectRef);

		KROLL_JAVASCRIPT_API void ProtectGlobalContext(JSGlobalContextRef);
		KROLL_JAVASCRIPT_API void UnprotectGlobalContext(JSGlobalContextRef);

		KROLL_JAVASCRIPT_API KValueRef Evaluate(JSContextRef context, const char* script);
		KROLL_JAVASCRIPT_API KValueRef EvaluateFile(JSContextRef context, std::string fullPath);
		KROLL_JAVASCRIPT_API void BindProperties(JSObjectRef, KObjectRef);
		KROLL_JAVASCRIPT_API KValueRef GetProperty(JSObjectRef, std::string name);
	};
}

#endif
