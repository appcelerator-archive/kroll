/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "javascript_module.h"
#include <Poco/FileStream.h>

namespace kroll
{
	JSClassRef tibo_class = NULL;
	JSClassRef tibm_class = NULL;
	JSClassRef tibl_class = NULL;
	const JSClassDefinition empty_class = { 0, 0, 0, 0, 0, 0,
	                                        0, 0, 0, 0, 0, 0,
	                                        0, 0, 0, 0, 0 };

	/* callback for KObject proxying to KJS */
	void get_property_names_cb(JSContextRef, JSObjectRef, JSPropertyNameAccumulatorRef);
	bool has_property_cb(JSContextRef, JSObjectRef, JSStringRef);
	JSValueRef get_property_cb(JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);
	bool set_property_cb(JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);
	JSValueRef call_as_function_cb(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	void finalize_cb(JSObjectRef);
	JSValueRef to_string_cb(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	JSValueRef equals_cb(JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);

	void AddSpecialPropertyNames(SharedValue, SharedStringList, bool);
	JSValueRef GetSpecialProperty(SharedValue, char*, JSContextRef, SharedValue);
	bool DoSpecialSetBehavior(SharedValue target, char* name, SharedValue newValue);

	SharedValue KJSUtil::ToKrollValue(
		JSValueRef value,
		JSContextRef ctx,
		JSObjectRef this_obj)
	{
		SharedValue krollValue = 0;
		JSValueRef exception = NULL;

		if (value == NULL)
		{
			Logger::Get("JavaScript.KJSUtil")->Error("Trying to convert NULL JSValueRef");
			return Value::Undefined;
		}

		if (JSValueIsNumber(ctx, value))
		{
			krollValue = Value::NewDouble(JSValueToNumber(ctx, value, &exception));
		}
		else if (JSValueIsBoolean(ctx, value))
		{
			krollValue = Value::NewBool(JSValueToBoolean(ctx, value));
		}
		else if (JSValueIsString(ctx, value))
		{
			JSStringRef string_ref = JSValueToStringCopy(ctx, value, &exception);
			if (string_ref)
			{
				char* chars = KJSUtil::ToChars(string_ref);
				std::string to_ret = std::string(chars);
				JSStringRelease(string_ref);
				free(chars);
				krollValue = Value::NewString(to_ret);
			}

		}
		else if (JSValueIsObject(ctx, value))
		{
			JSObjectRef o = JSValueToObject(ctx, value, &exception);
			if (o != NULL)
			{
				SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(o));
				if (value != NULL)
				{
					// This is a KJS-wrapped Kroll value: unwrap it
					return *value;
				}
				else if (JSObjectIsFunction(ctx, o))
				{
					// this is a pure JS method: proxy it
					SharedKMethod tibm = new KKJSMethod(ctx, o, this_obj);
					krollValue = Value::NewMethod(tibm);
				}
				else if (IsArrayLike(o, ctx))
				{
					// this is a pure JS array: proxy it
					SharedKList tibl = new KKJSList(ctx, o);
					krollValue = Value::NewList(tibl);
				}
				else
				{
					// this is a pure JS object: proxy it
					SharedKObject tibo = new KKJSObject(ctx, o);
					krollValue = Value::NewObject(tibo);
				}
			}
		}
		else if (JSValueIsNull(ctx, value))
		{
			krollValue = kroll::Value::Null;
		}
		else
		{
			krollValue = kroll::Value::Undefined;
		}
		if (!krollValue.isNull() && exception == NULL)
		{
			return krollValue;
		}
		else if (exception != NULL)
		{
			throw KJSUtil::ToKrollValue(exception, ctx, NULL);
		}
		else
		{
			Logger::Get("JavaScript.KJSUtil")->Error("Failed Kroll->JS conversion with no exception!");
			throw ValueException::FromString("Conversion from Kroll value to JS value failed");
		}
	}

	JSValueRef KJSUtil::ToJSValue(SharedValue value, JSContextRef ctx)
	{
		JSValueRef jsValue = NULL;
		if (value->IsInt())
		{
			jsValue = JSValueMakeNumber(ctx, value->ToInt());
		}
		else if (value->IsDouble())
		{
			jsValue = JSValueMakeNumber(ctx, value->ToDouble());
		}
		else if (value->IsBool())
		{
			jsValue = JSValueMakeBoolean(ctx, value->ToBool());
		}
		else if (value->IsString())
		{
			JSStringRef s = JSStringCreateWithUTF8CString(value->ToString());
			jsValue = JSValueMakeString(ctx, s);
			JSStringRelease(s);
		}
		else if (value->IsObject())
		{
			SharedKObject obj = value->ToObject();
			AutoPtr<KKJSObject> kobj = obj.cast<KKJSObject>();
			if (!kobj.isNull() && kobj->SameContextGroup(ctx))
			{
				// this object is actually a pure JS object
				jsValue = kobj->GetJSObject();
			}
			else
			{
				// this is a KObject that needs to be proxied
				jsValue = KJSUtil::KObjectToJSValue(value, ctx);
			}
		}
		else if (value->IsMethod())
		{
			SharedKMethod meth = value->ToMethod();
			AutoPtr<KKJSMethod> kmeth = meth.cast<KKJSMethod>();
			if (!kmeth.isNull() && kmeth->SameContextGroup(ctx))
			{
				// this object is actually a pure JS callable object
				jsValue = kmeth->GetJSObject();
			}
			else
			{
				// this is a KMethod that needs to be proxied
				jsValue = KJSUtil::KMethodToJSValue(value, ctx);
			}
		}
		else if (value->IsList())
		{
			SharedKList list = value->ToList();
			AutoPtr<KKJSList> klist = list.cast<KKJSList>();
			if (!klist.isNull() && klist->SameContextGroup(ctx))
			{
				// this object is actually a pure JS array
				jsValue = klist->GetJSObject();
			}
			else
			{
				// this is a KList that needs to be proxied
				jsValue = KJSUtil::KListToJSValue(value, ctx);
			}
		}
		else if (value->IsNull())
		{
			jsValue = JSValueMakeNull(ctx);
		}
		else if (value->IsUndefined())
		{
			jsValue = JSValueMakeUndefined(ctx);
		}
		else
		{
			jsValue = JSValueMakeUndefined(ctx);
		}

		return jsValue;

	}

	JSValueRef KJSUtil::KObjectToJSValue(SharedValue obj_val, JSContextRef c)
	{
		if (tibo_class == NULL)
		{
			JSClassDefinition js_class_def = empty_class;
			js_class_def.className = "Object";
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			tibo_class = JSClassCreate(&js_class_def);
		}
		return JSObjectMake(c, tibo_class, new SharedValue(obj_val));
	}

	JSValueRef KJSUtil::KMethodToJSValue(SharedValue meth_val, JSContextRef c)
	{
		if (tibm_class == NULL)
		{
			JSClassDefinition js_class_def = empty_class;
			js_class_def.className = "Function";
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			js_class_def.callAsFunction = call_as_function_cb;
			tibm_class = JSClassCreate(&js_class_def);
		}
		JSObjectRef ref = JSObjectMake(c, tibm_class, new SharedValue(meth_val));
		JSValueRef fnProtoValue = GetFunctionPrototype(c, NULL);
		JSObjectSetPrototype(c, ref, fnProtoValue);
		return ref;
	}

	void inline CopyJSProperty(
		JSContextRef c,
		JSObjectRef from_obj,
		SharedKObject to_bo,
		JSObjectRef to_obj,
		const char *prop_name)
	{

		JSStringRef prop_name_str = JSStringCreateWithUTF8CString(prop_name);
		JSValueRef prop = JSObjectGetProperty(c, from_obj, prop_name_str, NULL);
		JSStringRelease(prop_name_str);
		SharedValue prop_val = KJSUtil::ToKrollValue(prop, c, to_obj);
		to_bo->Set(prop_name, prop_val);
	}

	JSValueRef KJSUtil::KListToJSValue(SharedValue list_val, JSContextRef c)
	{

		if (tibl_class == NULL)
		{
			JSClassDefinition js_class_def = empty_class;
			js_class_def.className = "Array";
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			tibl_class = JSClassCreate(&js_class_def);
		}

		JSObjectRef ref = JSObjectMake(c, tibl_class, new SharedValue(list_val));
		JSValueRef aProtoValue = GetArrayPrototype(c, NULL);
		JSObjectSetPrototype(c, ref, aProtoValue);
		return ref;
	}

	char* KJSUtil::ToChars(JSStringRef js_string)
	{
		size_t size = JSStringGetMaximumUTF8CStringSize(js_string);
		char* string = (char*) malloc(size);
		JSStringGetUTF8CString(js_string, string, size);
		return string;
	}

	bool KJSUtil::IsArrayLike(JSObjectRef object, JSContextRef c)
	{
		bool array_like = true;

		JSStringRef pop = JSStringCreateWithUTF8CString("pop");
		array_like = array_like && JSObjectHasProperty(c, object, pop);
		JSStringRelease(pop);

		JSStringRef concat = JSStringCreateWithUTF8CString("concat");
		array_like = array_like && JSObjectHasProperty(c, object, concat);
		JSStringRelease(concat);

		JSStringRef length = JSStringCreateWithUTF8CString("length");
		array_like = array_like && JSObjectHasProperty(c, object, length);
		JSStringRelease(length);

		return array_like;
	}

	void finalize_cb(JSObjectRef js_object)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_object));
		delete value;
	}

	static bool PrototypeHasFunctionNamed(JSContextRef context, JSObjectRef object, JSStringRef name)
	{
		JSValueRef exception = NULL;

		JSValueRef prototypeValue = JSObjectGetPrototype(context, object);
		JSObjectRef prototype = JSValueToObject(context, prototypeValue, &exception);

		if (exception)
			return false;

		JSValueRef propValue = JSObjectGetProperty(context, prototype, name, &exception);
		if (exception)
			return false;

		if (!JSValueIsObject(context, propValue))
			return false;

		JSObjectRef prop = JSValueToObject(context, propValue, &exception);
		return !exception && JSObjectIsFunction(context, prop);
	}

	bool has_property_cb(
		JSContextRef js_context,
		JSObjectRef js_object,
		JSStringRef js_property)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_object));
		if (value == NULL)
			return false;

		// Convert the name to a std::string.
		SharedKObject object = (*value)->ToObject();
		char *name = KJSUtil::ToChars(js_property);
		std::string strName(name);
		free(name);

		// Special properties always take precedence. This is important
		// because even though the Array and Function prototypes have 
		// methods like toString -- we always want our special properties
		// to override those.
		SharedStringList specialProperties(new StringList());
		AddSpecialPropertyNames(*value, specialProperties, true);
		for (size_t i = 0; i < specialProperties->size(); i++)
		{
			if (strName == *specialProperties->at(i))
				return true;
		}

		// If the JavaScript prototype for Lists (Array) or Methods (Function) has
		// a method with the same name -- opt to use the prototype's version instead.
		// This will prevent  incompatible versions of things like pop() bleeding into
		// JavaScript.
		if (((*value)->IsList() || (*value)->IsMethod()) &&
			PrototypeHasFunctionNamed(js_context, js_object, js_property))
		{
			return false;
		}

		return object->HasProperty(strName.c_str());
	}

	JSValueRef get_property_cb(
		JSContextRef js_context,
		JSObjectRef js_object,
		JSStringRef js_property,
		JSValueRef* js_exception)
	{

		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_object));
		if (value == NULL)
			return JSValueMakeUndefined(js_context);

		SharedKObject object = (*value)->ToObject();
		char* name = KJSUtil::ToChars(js_property);
		JSValueRef jsValue = NULL;
		try
		{
			SharedValue ti_val = object->Get(name);
			jsValue = GetSpecialProperty(*value, name, js_context, ti_val);
		}
		catch (ValueException& exception)
		{
			*js_exception = KJSUtil::ToJSValue(exception.GetValue(), js_context);
		}
		catch (std::exception &e)
		{
			SharedValue v = Value::NewString(e.what());
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}
		catch (...)
		{
			std::string error = "Unknown exception trying to get property: ";
			error.append(name);
			SharedValue v = Value::NewString(error);
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}

		free(name);
		return jsValue;
	}

	bool set_property_cb(
		JSContextRef js_context,
		JSObjectRef js_object,
		JSStringRef js_property,
		JSValueRef js_value,
		JSValueRef* js_exception)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_object));
		if (value == NULL)
			return false;

		SharedKObject object = (*value)->ToObject();
		bool success = false;
		char* propertyName = KJSUtil::ToChars(js_property);
		try
		{
			SharedValue newValue = KJSUtil::ToKrollValue(js_value, js_context, js_object);

			// Arrays in particular have a special behavior when
			// you do something like set the "length" property
			if (!DoSpecialSetBehavior(*value, propertyName, newValue))
			{
				object->Set(propertyName, newValue);
			}
			success = true;
		}
		catch (ValueException& exception)
		{
			*js_exception = KJSUtil::ToJSValue(exception.GetValue(), js_context);
		}
		catch (std::exception &e)
		{
			SharedValue v = Value::NewString(e.what());
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}
		catch (...)
		{
			std::string error = "Unknown exception trying to set property: ";
			error.append(propertyName);
			SharedValue v = Value::NewString(error);
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}

		free(propertyName);
		return success;
	}

	JSValueRef call_as_function_cb(
		JSContextRef js_context,
		JSObjectRef js_function,
		JSObjectRef js_this,
		size_t num_args,
		const JSValueRef js_args[],
		JSValueRef* js_exception)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_function));
		if (value == NULL)
			return JSValueMakeUndefined(js_context);

		SharedKMethod method = (*value)->ToMethod();
		ValueList args;
		for (size_t i = 0; i < num_args; i++) {
			SharedValue argVal = KJSUtil::ToKrollValue(js_args[i], js_context, js_this);
			Value::Unwrap(argVal);
			args.push_back(argVal);
		}

		JSValueRef js_val = NULL;
		try
		{
			SharedValue ti_val = method->Call(args);
			js_val = KJSUtil::ToJSValue(ti_val, js_context);
		}
		catch (ValueException& exception)
		 {
			SharedString str = exception.DisplayString();
			*js_exception = KJSUtil::ToJSValue(exception.GetValue(), js_context);
		} 
		catch (std::exception &e)
		{
			SharedValue v = Value::NewString(e.what());
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}
		catch (...)
		{
			SharedValue v = Value::NewString("Unknown exception during Kroll method call");
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}

		return js_val;
	}

	void AddSpecialPropertyNames(SharedValue value, SharedStringList props, bool showInvisible)
	{
		// Some attributes should be hidden unless the are requested specifically -- 
		// essentially a has_property(...) versus  get_property_list(...). An example
		// of this type of attribute is toString(). Some JavaScript code might expect
		// a "hash" object to have no methods in its property list. We don't want
		// toString() to show up in those situations.

		bool foundLength = false, foundToString = false, foundEquals = false;
		for (size_t i = 0; i < props->size(); i++)
		{
			SharedString pn = props->at(i);
			if (strcmp(pn->c_str(), "length") == 0)
				foundLength = true;
			if (strcmp(pn->c_str(), "toString") == 0)
				foundToString = true;
			if (strcmp(pn->c_str(), "equals") == 0)
				foundEquals = true;
		}

		if (!foundLength && value->IsList())
		{
			props->push_back(new std::string("length"));
		}

		if (!foundToString && showInvisible)
		{
			props->push_back(new std::string("toString"));
		}

		if (!foundEquals && showInvisible)
		{
			props->push_back(new std::string("equals"));
		}
	}

	JSValueRef GetSpecialProperty(SharedValue value, char* name, JSContextRef ctx, SharedValue objValue)
	{
		// Always override the length property on lists. Some languages
		// supply their own length property, which might be a method instead
		// of a number -- bad news.
		if (value->IsList() && !strcmp(name, "length"))
		{
			SharedKList l = value->ToList();
			return JSValueMakeNumber(ctx, l->Size());
		}

		// Only overload these methods if the value in our object is not a
		// method We want the user to be able to supply their own versions,
		// but we don't want JavaScript code to freak out in situations where
		// Kroll objects use attributes with the same name that aren't methods.
		if (!objValue->IsMethod())
		{
			if (!strcmp(name, "toString"))
			{
				JSStringRef s = JSStringCreateWithUTF8CString("toString");
				return JSObjectMakeFunctionWithCallback(ctx, s, &to_string_cb);
			}

			if (!strcmp(name, "equals"))
			{
				JSStringRef s = JSStringCreateWithUTF8CString("equals");
				return JSObjectMakeFunctionWithCallback(ctx, s, &equals_cb);
			}
		}

		// Otherwise this is just a normal JS value
		return KJSUtil::ToJSValue(objValue, ctx);
	}

	bool DoSpecialSetBehavior(SharedValue target, char* name, SharedValue newValue)
	{
		// We only do something special if we are trying to set
		// the length property of a list to a new int value.
		if (strcmp(name, "length") || !target->IsList() || !newValue->IsInt())
		{
			return false;
		}
		target->ToList()->ResizeTo(newValue->ToInt());
		return true;
	}

	JSValueRef to_string_cb(
		JSContextRef js_context,
		JSObjectRef js_function,
		JSObjectRef js_this,
		size_t num_args,
		const JSValueRef args[],
		JSValueRef* exception)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_this));
		if (value == NULL)
			return JSValueMakeUndefined(js_context);

		SharedString ss = (*value)->DisplayString(2);
		SharedValue dsv = Value::NewString(ss);
		return KJSUtil::ToJSValue(dsv, js_context);
	}

	JSValueRef equals_cb(
		JSContextRef ctx,
		JSObjectRef function,
		JSObjectRef jsThis,
		size_t numArgs,
		const JSValueRef args[],
		JSValueRef* exception)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(jsThis));
		if (value == NULL || numArgs < 1)
		{
			return JSValueMakeBoolean(ctx, false);
		}

		// Ensure argument is a JavaScript object
		if (!JSValueIsObject(ctx, args[0]))
		{
			return JSValueMakeBoolean(ctx, false);
		}

		// Ensure argument is a Kroll JavaScript
		JSObjectRef otherObject = JSValueToObject(ctx, args[0], NULL);
		SharedValue* otherValue = static_cast<SharedValue*>(JSObjectGetPrivate(otherObject));
		if (otherValue == NULL)
		{
			return JSValueMakeBoolean(ctx, false);
		}

		// Test equality
		return JSValueMakeBoolean(ctx, (*value)->Equals(*otherValue));
	}

	void get_property_names_cb(
		JSContextRef js_context,
		JSObjectRef js_object,
		JSPropertyNameAccumulatorRef js_properties)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_object));
		if (value == NULL)
			return;

		SharedKObject object = (*value)->ToObject();
		SharedStringList props = object->GetPropertyNames();
		AddSpecialPropertyNames(*value, props, false);
		for (size_t i = 0; i < props->size(); i++)
		{
			SharedString pn = props->at(i);
			JSStringRef name = JSStringCreateWithUTF8CString(pn->c_str());
			JSPropertyNameAccumulatorAddName(js_properties, name);
			JSStringRelease(name);
		}
	}
	
	JSObjectRef KJSUtil::CreateNewGlobalContext(Host *host, bool add_global_object)
	{
		JSGlobalContextRef context = JSGlobalContextCreate(NULL);
		JSObjectRef global_object = JSContextGetGlobalObject(context);
		KJSUtil::RegisterGlobalContext(global_object, context);
		
		if (add_global_object)
		{

			/* Take some steps to insert the API into the Javascript context */
			/* Create a crazy, crunktown delegate hybrid object for Javascript */
			SharedValue global_value = Value::NewObject(host->GetGlobalObject());

			/* convert JS API to a KJS object */
			JSValueRef js_api = KJSUtil::ToJSValue(global_value, context);

			/* set the API as a property of the global object */
			JSStringRef prop_name = JSStringCreateWithUTF8CString(PRODUCT_NAME);
			JSObjectSetProperty(context, global_object, prop_name,
			                    js_api, kJSPropertyAttributeNone, NULL);

		}
		
		return global_object;
	}
	
	std::map<JSObjectRef, JSGlobalContextRef> KJSUtil::contextMap;
	void KJSUtil::RegisterGlobalContext(
		JSObjectRef object,
		JSGlobalContextRef globalContext)
	{
		contextMap[object] = globalContext;
	}
	
	void KJSUtil::UnregisterGlobalContext(JSObjectRef object)
	{
		std::map<JSObjectRef, JSGlobalContextRef>::iterator i = contextMap.find(object);
		if (i!=contextMap.end())
		{
			contextMap.erase(i);
		}
	}
	

	JSGlobalContextRef KJSUtil::GetGlobalContext(JSObjectRef object)
	{
		if (contextMap.find(object) == contextMap.end())
		{
			return NULL;
		}
		else
		{
			return contextMap[object];
		}
	}

	std::map<JSGlobalContextRef, int> KJSUtil::contextRefCounts;
	void KJSUtil::ProtectGlobalContext(JSGlobalContextRef globalContext)
	{
		if (contextRefCounts.find(globalContext) == contextRefCounts.end())
		{
			JSGlobalContextRetain(globalContext);
			contextRefCounts[globalContext] = 1;
		}
		else
		{
			contextRefCounts[globalContext]++;
		}
	}

	void KJSUtil::UnprotectGlobalContext(JSGlobalContextRef globalContext)
	{
		std::map<JSGlobalContextRef, int>::iterator i
			= contextRefCounts.find(globalContext);

		if (i == contextRefCounts.end())
		{
			Logger::Get("JavaScript.KJSUtil")->Error("Tried to unprotect an unprotected context!");
		}
		else if (i->second == 1)
		{
			JSGlobalContextRelease(globalContext);
			contextRefCounts.erase(i);
		}
		else
		{
			contextRefCounts[globalContext]--;
		}
	}

	SharedValue KJSUtil::Evaluate(JSContextRef context, char *script)
	{
		JSObjectRef global_object = JSContextGetGlobalObject(context);
		JSStringRef script_contents = JSStringCreateWithUTF8CString(script);
		JSStringRef url = JSStringCreateWithUTF8CString("<string>");
		JSValueRef exception = NULL;

		JSValueRef return_value = JSEvaluateScript(context, script_contents, global_object, url, 0, &exception);

		JSStringRelease(url);
		JSStringRelease(script_contents);

		if (exception != NULL) {
			throw KJSUtil::ToKrollValue(exception, context, NULL);
		}

		return ToKrollValue(return_value, context, global_object);
	}

	SharedValue KJSUtil::EvaluateFile(JSContextRef context, char *full_path)
	{
		PRINTD("Evaluating JS: " << full_path);
		JSObjectRef global_object = JSContextGetGlobalObject(context);
		Poco::FileInputStream* script_stream = new Poco::FileInputStream(full_path, std::ios::in);
		std::string script_contents;
		while (!script_stream->eof())
		{
			std::string s;
			std::getline(*script_stream, s);

			script_contents.append(s);
			script_contents.append("\n");
		}
		script_stream->close();
		
		JSStringRef script = JSStringCreateWithUTF8CString(script_contents.c_str());
		JSStringRef full_path_url = JSStringCreateWithUTF8CString(full_path);
		JSValueRef exception = NULL;
		JSValueRef return_value = JSEvaluateScript(context, script, global_object, full_path_url, 0, &exception);
		JSStringRelease(script);
		JSStringRelease(full_path_url);
		delete script_stream;

		if (exception != NULL) {
			SharedValue v = KJSUtil::ToKrollValue(exception, context, NULL);
			throw ValueException(v);
		}

		return KJSUtil::ToKrollValue(return_value, context, global_object);
	}

	//===========================================================================//
	// METHODS BORROWED ARE TAKEN FROM GWT - modifications under same license
	//===========================================================================//
	/*
	 * Copyright 2008 Google Inc.
	 * 
	 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
	 * use this file except in compliance with the License. You may obtain a copy of
	 * the License at
	 * 
	 * http://www.apache.org/licenses/LICENSE-2.0
	 * 
	 * Unless required by applicable law or agreed to in writing, software
	 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
	 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
	 * License for the specific language governing permissions and limitations under
	 * the License.
	 */
	
	/*
	 * The following takes the prototype from the Function constructor, this allows
	 * us to easily support call and apply on our objects that support CallAsFunction.
	 *
	 * NOTE: The return value is not protected.
	 */
	JSValueRef KJSUtil::GetFunctionPrototype(JSContextRef jsContext, JSValueRef* exception) 
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef fnPropName = JSStringCreateWithUTF8CString("Function");
		JSValueRef fnCtorValue = JSObjectGetProperty(jsContext, globalObject,
			fnPropName, exception);
		JSStringRelease(fnPropName);
		if (!fnCtorValue)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSObjectRef fnCtorObject = JSValueToObject(jsContext, fnCtorValue, exception);
		if (!fnCtorObject)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSStringRef protoPropName = JSStringCreateWithUTF8CString("prototype");
		JSValueRef fnPrototype = JSObjectGetProperty(jsContext, fnCtorObject,
			protoPropName, exception);
		JSStringRelease(protoPropName);
		if (!fnPrototype)
		{
			return JSValueMakeUndefined(jsContext);
		}

	return fnPrototype;
	}

	/*
	 * The following takes the prototype from the Array constructor, this allows
	 * us to easily support array like functions
	 *
	 * NOTE: The return value is not protected.
	 */
	JSValueRef KJSUtil::GetArrayPrototype(JSContextRef jsContext, JSValueRef* exception) 
	{
		JSObjectRef globalObject = JSContextGetGlobalObject(jsContext);
		JSStringRef fnPropName = JSStringCreateWithUTF8CString("Array");
		JSValueRef fnCtorValue = JSObjectGetProperty(jsContext, globalObject,
			fnPropName, exception);
		JSStringRelease(fnPropName);
		if (!fnCtorValue) 
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSObjectRef fnCtorObject = JSValueToObject(jsContext, fnCtorValue, exception);
		if (!fnCtorObject)
		{
			return JSValueMakeUndefined(jsContext);
		}

		JSStringRef protoPropName = JSStringCreateWithUTF8CString("prototype");
		JSValueRef fnPrototype = JSObjectGetProperty(jsContext, fnCtorObject,
			protoPropName, exception);
		JSStringRelease(protoPropName);
		if (!fnPrototype) 
		{
			return JSValueMakeUndefined(jsContext);
		}

		return fnPrototype;
	}
	
	void KJSUtil::BindProperties(JSObjectRef global_object, SharedKObject obj)
	{
		JSGlobalContextRef context = GetGlobalContext(global_object);

		SharedStringList names = obj->GetPropertyNames();
		for (size_t i = 0; i < names->size(); i++)
		{
			std::string other = *names->at(i);
			SharedValue v = obj->Get(other.c_str());
			JSValueRef js = KJSUtil::ToJSValue(v, context);
			JSStringRef prop_name = JSStringCreateWithUTF8CString(other.c_str());
			JSObjectSetProperty(context, global_object, prop_name, js, kJSPropertyAttributeNone, NULL);
		}
	}
	
	SharedValue KJSUtil::GetProperty(JSObjectRef global_object, std::string name)
	{
		JSGlobalContextRef context = GetGlobalContext(global_object);
		JSStringRef prop_name_str = JSStringCreateWithUTF8CString(name.c_str());
		JSValueRef prop = JSObjectGetProperty(context, global_object, prop_name_str, NULL);
		JSStringRelease(prop_name_str);
		return KJSUtil::ToKrollValue(prop, context, global_object);
	}
	
}
