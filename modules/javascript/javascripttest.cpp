/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascripttest.h"
#include "kjs.h"

namespace kroll
{
	void JavascriptUnitTestSuite::Run(Host *host)
	{
		// create a javascript environment
		JSGlobalContextRef context = JSGlobalContextCreate(NULL);
		KR_ASSERT(context);
		JSObjectRef globalObject = JSContextGetGlobalObject(context);
		KR_ASSERT(globalObject);
		
		// TEST simple bound property values
		BoundObject *bound = new StaticBoundObject();
		Value *value = new Value("bar");
		bound->Set("foo",value);
		KR_DECREF(value);
		JSObjectRef boundRef = KrollBoundObjectToJSValue(context, bound);
		KR_ASSERT(boundRef);
		
		// get the value of the property from the JS object
		JSStringRef key = JSStringCreateWithUTF8CString("foo");
		JSValueRef js_value = JSObjectGetProperty(context,
		                        boundRef,
		                        key,
		                        NULL);
		JSStringRelease(key);
		KR_ASSERT(js_value);

		// convert to a js string and then a char* so we can make sure its the same
		JSStringRef valueAsString = JSValueToStringCopy(context, js_value, NULL);
		size_t jsSize = JSStringGetMaximumUTF8CStringSize(valueAsString);
		char jsBuffer[jsSize];
		JSStringGetUTF8CString(valueAsString, jsBuffer, jsSize);
		JSStringRelease(valueAsString);

		KR_ASSERT_STR(jsBuffer,"bar");
		
		KR_DECREF(bound);
		
		// tear it down
		JSGlobalContextRelease(context);
		JSGarbageCollect(context);
	}
}
