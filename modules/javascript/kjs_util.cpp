
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

	SharedValue KJSUtil::ToKrollValue(
		JSValueRef value,
		JSContextRef ctx,
		JSObjectRef this_obj)
	{
		SharedValue kr_val;
		JSValueRef exception = NULL;

		if (value == NULL)
		{
			std::cerr << "Trying to convert NULL JSValueRef!" << std::endl;
			return Value::Undefined;
		}

		if (JSValueIsNumber(ctx, value))
		{
			kr_val = Value::NewDouble(JSValueToNumber(ctx, value, &exception));
		}
		else if (JSValueIsBoolean(ctx, value))
		{
			kr_val = Value::NewBool(JSValueToBoolean(ctx, value));
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
				kr_val = Value::NewString(to_ret);
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
					SharedKMethod tibm = new KJSKMethod(ctx, o, this_obj);
					kr_val = Value::NewMethod(tibm);
				}
				else if (IsArrayLike(o, ctx))
				{
					// this is a pure JS array: proxy it
					SharedKList tibl = new KJSKList(ctx, o);
					kr_val = Value::NewList(tibl);
				}
				else
				{
					// this is a pure JS object: proxy it
					SharedKObject tibo = new KJSKObject(ctx, o);
					kr_val = Value::NewObject(tibo);
				}
			}
		}
		else if (JSValueIsNull(ctx, value))
		{
			kr_val = kroll::Value::Null;
		}
		else
		{
			kr_val = kroll::Value::Undefined;
		}

		if (!kr_val.isNull() && exception == NULL)
		{
			return kr_val;
		}
		else
		{
			throw KJSUtil::ToKrollValue(exception, ctx, NULL);
		}
	}

	JSValueRef KJSUtil::ToJSValue(SharedValue value, JSContextRef ctx)
	{
		JSValueRef js_val;
		if (value->IsInt())
		{
			js_val = JSValueMakeNumber(ctx, value->ToInt());
		}
		else if (value->IsDouble())
		{
			js_val = JSValueMakeNumber(ctx, value->ToDouble());
		}
		else if (value->IsBool())
		{
			js_val = JSValueMakeBoolean(ctx, value->ToBool());
		}
		else if (value->IsString())
		{
			JSStringRef s = JSStringCreateWithUTF8CString(value->ToString());
			js_val = JSValueMakeString(ctx, s);
			JSStringRelease(s);
		}
		else if (value->IsObject())
		{
			SharedKObject obj = value->ToObject();
			SharedPtr<KJSKObject> kobj = obj.cast<KJSKObject>();
			if (!kobj.isNull() && kobj->SameContextGroup(ctx))
			{
				// this object is actually a pure JS object
				js_val = kobj->GetJSObject();
			}
			else
			{
				// this is a KObject that needs to be proxied
				js_val = KJSUtil::KObjectToJSValue(value, ctx);
			}
		}
		else if (value->IsMethod())
		{
			SharedKMethod meth = value->ToMethod();
			SharedPtr<KJSKMethod> kmeth = meth.cast<KJSKMethod>();
			if (!kmeth.isNull() && kmeth->SameContextGroup(ctx))
			{
				// this object is actually a pure JS callable object
				js_val = kmeth->GetJSObject();
			}
			else
			{
				// this is a KMethod that needs to be proxied
				js_val = KJSUtil::KMethodToJSValue(value, ctx);
			}
		}
		else if (value->IsList())
		{
			SharedKList list = value->ToList();
			SharedPtr<KJSKList> klist = list.cast<KJSKList>();
			if (!klist.isNull() && klist->SameContextGroup(ctx))
			{
				// this object is actually a pure JS array
				js_val = klist->GetJSObject();
			}
			else
			{
				// this is a KList that needs to be proxied
				js_val = KJSUtil::KListToJSValue(value, ctx);
			}
		}
		else if (value->IsNull())
		{
			js_val = JSValueMakeNull(ctx);
		}
		else if (value->IsUndefined())
		{
			js_val = JSValueMakeUndefined(ctx);
		}
		else
		{
			js_val = JSValueMakeUndefined(ctx);
		}

		return js_val;

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
			js_class_def.className = "KMethod";
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			js_class_def.callAsFunction = call_as_function_cb;
			tibm_class = JSClassCreate(&js_class_def);
		}
		return JSObjectMake(c, tibm_class, new SharedValue(meth_val));
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
			js_class_def.className = "KList";
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			tibl_class = JSClassCreate(&js_class_def);
		}

		JSObjectRef object = JSObjectMake(c, tibl_class, new SharedValue(list_val));

		JSValueRef args[1] = { JSValueMakeNumber(c, 3) };
		JSObjectRef array = JSObjectMakeArray(c, 1, args, NULL);

		/* we are doing this manually because there have been
		   propblems trying to set an object's prototype to array */

		// move some array methods
		SharedKList list = list_val->ToList();
		CopyJSProperty(c, array, list, object, "push");
		CopyJSProperty(c, array, list, object, "pop");
		CopyJSProperty(c, array, list, object, "shift");
		CopyJSProperty(c, array, list, object, "unshift");
		CopyJSProperty(c, array, list, object, "reverse");
		CopyJSProperty(c, array, list, object, "splice");
		CopyJSProperty(c, array, list, object, "join");
		CopyJSProperty(c, array, list, object, "slice");
		CopyJSProperty(c, array, list, object, "concat");

		return object;
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
		bool found_length = false;
		for (size_t i = 0; i < props->size(); i++)
		{
			SharedString pn = props->at(i);
			JSStringRef name = JSStringCreateWithUTF8CString(pn->c_str());
			JSPropertyNameAccumulatorAddName(js_properties, name);
			JSStringRelease(name);
			if (strcmp(pn->c_str(), "length") == 0)
				found_length = true;
		}

		if (!found_length && (*value)->IsList())
		{
			JSStringRef name = JSStringCreateWithUTF8CString("length");
			JSPropertyNameAccumulatorAddName(js_properties, name);
			JSStringRelease(name);
		}
	}

	bool has_property_cb(
		JSContextRef js_context,
		JSObjectRef js_object,
		JSStringRef js_property)
	{
		SharedValue* value = static_cast<SharedValue*>(JSObjectGetPrivate(js_object));
		if (value == NULL)
			return false;

		SharedKObject object = (*value)->ToObject();
		char *name = KJSUtil::ToChars(js_property);
		std::string str_name(name);
		free(name);

		SharedStringList names = object->GetPropertyNames();
		for (size_t i = 0; i < names->size(); i++)
		{
			if (str_name == *names->at(i)) {
				return true;
			}
		}

		// Fake the length property for lists
		if ((*value)->IsList() && str_name == std::string("length"))
			return true;

		return false;
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
		JSValueRef js_val = NULL;
		try
		{
			SharedValue ti_val = object->Get(name);

			// Fake the length property for lists
			if ((*value)->IsList() &&
				strcmp(name, "length") == 0 &&
				ti_val->IsUndefined())
				ti_val = Value::NewInt((*value)->ToList()->Size());

			js_val = KJSUtil::ToJSValue(ti_val, js_context);
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
			std::cerr << "KJSUtil.cpp: Caught an unknown exception during get for "
			          << name << std::endl;
			SharedValue v = Value::NewString("unknown exception");
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}

		free(name);
		return js_val;
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
		char* prop_name = KJSUtil::ToChars(js_property);
		try
		{
			SharedValue ti_val = KJSUtil::ToKrollValue(js_value, js_context, js_object);
			object->Set(prop_name, ti_val);
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
			std::cerr << "KJSUtil.cpp: Caught an unknown exception during set for "
			          << prop_name << std::endl;
			SharedValue v = Value::NewString("unknown exception");
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}

		free(prop_name);
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
			SharedValue arg_val = KJSUtil::ToKrollValue(js_args[i], js_context, js_this);
			args.push_back(arg_val);
		}

		JSValueRef js_val = NULL;
		try
		{
			SharedValue ti_val = method->Call(args);
			js_val = KJSUtil::ToJSValue(ti_val, js_context);
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
			std::cerr << "KJSUtil.cpp: Caught an unknown exception during call()"
			          << std::endl;
			SharedValue v = Value::NewString("unknown exception");
			*js_exception = KJSUtil::ToJSValue(v, js_context);
		}

		return js_val;
	}

	SharedPtr<KJSKObject> KJSUtil::ToBoundObject(
		JSContextRef context,
		JSObjectRef object)
	{
		return new KJSKObject(context, object);
	}

	SharedPtr<KJSKMethod> KJSUtil::ToBoundMethod(
		JSContextRef context,
		JSObjectRef method,
		JSObjectRef this_object)
	{
		return new KJSKMethod(context, method, this_object);
	}

	SharedPtr<KJSKList> KJSUtil::ToBoundList(
		JSContextRef context,
		JSObjectRef list)
	{
		return new KJSKList(context, list);
	}

	std::map<JSObjectRef, JSGlobalContextRef> context_map;
	void KJSUtil::RegisterGlobalContext(
		JSObjectRef object,
		JSGlobalContextRef global_ctx)
	{
		context_map[object] = global_ctx;
	}

	JSGlobalContextRef KJSUtil::GetGlobalContext(JSObjectRef object)
	{
		if (context_map.find(object) == context_map.end())
		{
			return NULL;
		}
		else
		{
			return context_map[object];
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

			script_contents.append(s + "\n");
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
}
