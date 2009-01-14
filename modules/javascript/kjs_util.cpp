
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "javascript_module.h"

namespace kroll
{
	JSClassRef tibo_class = NULL;
	JSClassRef tibm_class = NULL;
	JSClassRef tibl_class = NULL;
	const JSClassDefinition empty_class = { 0, 0, 0, 0, 0, 0,
	                                        0, 0, 0, 0, 0, 0,
	                                        0, 0, 0, 0, 0 };
	/* callback for BoundObject proxying to KJS */
	void get_property_names_cb (JSContextRef, JSObjectRef, JSPropertyNameAccumulatorRef);
	bool has_property_cb (JSContextRef, JSObjectRef, JSStringRef);
	JSValueRef get_property_cb (JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);
	bool set_property_cb (JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);
	JSValueRef call_as_function_cb (JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
	void finalize_cb(JSObjectRef);

	Value* KJSUtil::ToKrollValue(JSValueRef value,
	                             JSContextRef ctx, 
	                             JSObjectRef this_obj)
	{

		Value *kr_val = NULL;
		JSValueRef exception = NULL;

		if (value == NULL)
		{
			std::cerr << "Trying to convert NULL JSValueRef!" << std::endl;
			return Value::Undefined();
		}

		if (JSValueIsNumber(ctx, value))
		{
			kr_val = new Value(JSValueToNumber(ctx, value, &exception));
		}
		else if (JSValueIsBoolean(ctx, value))
		{
			kr_val = new Value(JSValueToBoolean(ctx, value));
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
				kr_val = new Value(to_ret);
			}

		}
		else if (JSValueIsObject(ctx, value))
		{

			JSObjectRef o = JSValueToObject(ctx, value, &exception);
			if (o != NULL)
			{
				void* data = (void*) JSObjectGetPrivate(o);
				if (JSObjectIsFunction(ctx, o) && data == NULL)
				{
					// this is a pure JS method: proxy it
					BoundMethod* tibm = new KJSBoundMethod(ctx, o, this_obj);
					kr_val = new Value(tibm);
					KR_DECREF(tibm);
				}
				else if (JSObjectIsFunction(ctx, o))
				{
					// this is a TiBoundMethod: unwrap it
					BoundMethod* tibm = (BoundMethod*) data;
					kr_val = new Value(tibm);
				}
				//else if (KJSUtil::IsArrayLike(o, ctx) && data == NULL)
				//{
				//	// this is a pure JS array: proxy it
				//	BoundList* tibl = new KJSBoundList(ctx, o);
				//	kr_val = new Value(tibl);
				//	KR_DECREF(tibl);
				//}
				//else if (JSObjectIsArrayLike(ctx, o))
				//{
				//	// this is a TiBoundList: unwrap it
				//	BoundList* tibl = (BoundList*) data;
				//	kr_val = new Value(tibl);
				//}
				else if (data == NULL)
				{
					// this is a pure JS object: proxy it
					BoundObject* tibo = new KJSBoundObject(ctx, o);
					kr_val = new Value(tibo);
					KR_DECREF(tibo);
				}
				else
				{
					// this is a kroll::BoundObject: unwrap it
					BoundObject* tibo = (BoundObject*) data;
					kr_val = new Value(tibo);
				}
			}

		}
		else if (JSValueIsNull(ctx, value))
		{
			kr_val = kroll::Value::Null();
		}
		else
		{
			kr_val = kroll::Value::Undefined();
		}

		if (kr_val != NULL && exception == NULL)
		{
			return kr_val;
		}
		else
		{
			throw KJSUtil::ToKrollValue(exception, ctx, NULL);
		}
	}

	JSValueRef KJSUtil::ToJSValue(Value* value, JSContextRef ctx)
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
			JSStringRef s = JSStringCreateWithUTF8CString(value->ToString().c_str());
			js_val = JSValueMakeString(ctx, s);
			JSStringRelease(s);
		}
		else if (value->IsObject())
		{
			BoundObject* obj = value->ToObject();
			KJSBoundObject* kobj = dynamic_cast<KJSBoundObject*>(obj);
			if (kobj != NULL && kobj->SameContextGroup(ctx))
			{
				// this object is actually a pure JS object
				js_val = kobj->GetJSObject();
			}
			else
			{
				// this is a BoundObject that needs to be proxied
				js_val = KJSUtil::ToJSValue(obj, ctx);
			}
		}
		else if (value->IsMethod())
		{
			BoundMethod* meth = value->ToMethod();
			KJSBoundMethod* kmeth = dynamic_cast<KJSBoundMethod*>(meth);
			if (kmeth != NULL && kmeth->SameContextGroup(ctx))
			{
				// this object is actually a pure JS callable object
				js_val = kmeth->GetJSObject();
			}
			else
			{
				// this is a TiBoundMethod that needs to be proxied
				js_val = KJSUtil::ToJSValue(meth, ctx);
			}
		}
		else if (value->IsList())
		{
			BoundList* list = value->ToList();
			KJSBoundList* klist = dynamic_cast<KJSBoundList*>(list);
			if (klist != NULL && klist->SameContextGroup(ctx))
			{
				// this object is actually a pure JS array
				js_val = klist->GetJSObject();
			}
			else
			{
				// this is a TiBoundMethod that needs to be proxied
				js_val = KJSUtil::ToJSValue(list, ctx);
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

	JSValueRef KJSUtil::ToJSValue(BoundObject* object, JSContextRef c)
	{
		if (tibo_class == NULL)
		{
			JSClassDefinition js_class_def = empty_class;
			js_class_def.className = strdup("KrollBoundObject");
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			tibo_class = JSClassCreate (&js_class_def);
		}
		KR_ADDREF(object);
		return JSObjectMake (c, tibo_class, object);
	}

	JSValueRef KJSUtil::ToJSValue(BoundMethod* method, JSContextRef c)
	{
		if (tibm_class == NULL)
		{
			JSClassDefinition js_class_def = empty_class;
			js_class_def.className = strdup("KrollBoundMethod");
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			js_class_def.callAsFunction = call_as_function_cb;
			tibm_class = JSClassCreate (&js_class_def);
		}
		KR_ADDREF(method);
		return JSObjectMake (c, tibm_class, method);
	}

	void inline CopyJSProperty(JSContextRef c,
	                           JSObjectRef from_obj,
	                           BoundObject *to_bo,
	                           JSObjectRef to_obj,
	                           const char *prop_name)
	{

		JSStringRef prop_name_str = JSStringCreateWithUTF8CString(prop_name);
		JSValueRef prop = JSObjectGetProperty(c, from_obj, prop_name_str, NULL);
		JSStringRelease(prop_name_str);
		Value *prop_val = KJSUtil::ToKrollValue(prop, c, to_obj);
		ScopedDereferencer s(prop_val);
		to_bo->Set(prop_name, prop_val);
	}

	JSValueRef KJSUtil::ToJSValue(BoundList *list, JSContextRef c)
	{

		if (tibl_class == NULL)
		{
			JSClassDefinition js_class_def = empty_class;
			js_class_def.className = strdup("KrollBoundList");
			js_class_def.getPropertyNames = get_property_names_cb;
			js_class_def.finalize = finalize_cb;
			js_class_def.hasProperty = has_property_cb;
			js_class_def.getProperty = get_property_cb;
			js_class_def.setProperty = set_property_cb;
			tibl_class = JSClassCreate (&js_class_def);
		}

		KR_ADDREF(list);
		JSObjectRef object = JSObjectMake (c, tibl_class, list);

		JSValueRef args[1] = { JSValueMakeNumber(c, 3) };
		JSObjectRef array = JSObjectMakeArray(c, 1, args, NULL);

		/* we are doing this manually because there have been
		   propblems trying to set an object's prototype to array */

		// move some array methods
		CopyJSProperty(c, array, list, object, "push");
		CopyJSProperty(c, array, list, object, "pop");
		CopyJSProperty(c, array, list, object, "shift");
		CopyJSProperty(c, array, list, object, "unshift");
		CopyJSProperty(c, array, list, object, "reverse");
		CopyJSProperty(c, array, list, object, "splice");
		CopyJSProperty(c, array, list, object, "join");
		CopyJSProperty(c, array, list, object, "slice");

		return object;
	}

	char* KJSUtil::ToChars(JSStringRef js_string)
	{
		size_t size = JSStringGetMaximumUTF8CStringSize (js_string);
		char* string = (char*) malloc(size);
		JSStringGetUTF8CString(js_string, string, size);
		return string;
	}

	bool KJSUtil::IsArrayLike(JSObjectRef object, JSContextRef c)
	{
		return false;
	}

	void KJSUtil::BindPropertyToJSObject(JSContextRef ctx,
	                            JSObjectRef o,
	                            const char *name,
	                            JSValueRef property)
	{
		JSStringRef name_str = JSStringCreateWithUTF8CString(name);
		JSObjectSetProperty(ctx, o, name_str, property, kJSPropertyAttributeNone, NULL);
		JSStringRelease(name_str);
	}

	void finalize_cb(JSObjectRef js_object)
	{
		BoundObject* object = (BoundObject*) JSObjectGetPrivate (js_object);
		KR_DECREF(object);
	}

	void get_property_names_cb (JSContextRef js_context,
	                            JSObjectRef js_object,
	                            JSPropertyNameAccumulatorRef js_properties)
	{
		BoundObject* object = (BoundObject*) JSObjectGetPrivate (js_object);

		if (object == NULL)
			return;

		std::vector<const char *> props;
		object->GetPropertyNames(&props);
		for (size_t i = 0; i < props.size(); i++)
		{
			JSStringRef name = JSStringCreateWithUTF8CString(props.at(i));
			JSPropertyNameAccumulatorAddName(js_properties, name);
			JSStringRelease(name);
		}
	}

	bool has_property_cb (JSContextRef js_context,
	                      JSObjectRef  js_object,
	                      JSStringRef  js_property)
	{
		BoundObject* object = (BoundObject*) JSObjectGetPrivate (js_object);
		if (object == NULL)
			return false;

		char *name = KJSUtil::ToChars(js_property);
		std::string str_name(name);
		free(name);

		std::vector<const char *> *names = new std::vector<const char *>();
		object->GetPropertyNames(names);
		for (size_t i = 0; i < names->size(); i++)
		{
			if (str_name == names->at(i)) {
				return true;
			}
		}

		return false;
	}

	JSValueRef get_property_cb (JSContextRef js_context,
	                            JSObjectRef  js_object,
	                            JSStringRef  js_property,
	                            JSValueRef*  js_exception)
	{
		BoundObject* object = (BoundObject*) JSObjectGetPrivate (js_object);
		if (object == NULL)
			return JSValueMakeUndefined(js_context);

		JSValueRef js_val = NULL;
		char* name = KJSUtil::ToChars(js_property);
		try
		{
			Value* ti_val = object->Get(name);
			ScopedDereferencer s(ti_val);
			js_val = KJSUtil::ToJSValue(ti_val, js_context);
		}
		catch (Value* exception)
		{
			ScopedDereferencer s(exception);
			*js_exception = KJSUtil::ToJSValue(exception, js_context);
		}

		free(name);
		return js_val;
	}

	bool set_property_cb (JSContextRef js_context,
	                      JSObjectRef  js_object,
	                      JSStringRef  js_property,
	                      JSValueRef   js_value,
	                      JSValueRef*  js_exception)
	{
		BoundObject* object = (BoundObject*) JSObjectGetPrivate (js_object);
		if (object == NULL)
			return false;

		char* prop_name = KJSUtil::ToChars(js_property);
		try
		{
			Value* ti_val = KJSUtil::ToKrollValue(js_value, js_context, js_object);
			ScopedDereferencer s(ti_val);
			object->Set(prop_name, ti_val);
		}
		catch (Value* exception)
		{
			ScopedDereferencer s(exception);
			*js_exception = KJSUtil::ToJSValue(exception, js_context);
		}

		free(prop_name);
		return true;
	}

	JSValueRef call_as_function_cb (JSContextRef     js_context,
	                                JSObjectRef      js_function,
	                                JSObjectRef      js_this,
	                                size_t           num_args,
	                                const JSValueRef js_args[],
	                                JSValueRef*      js_exception)
	{
		BoundMethod* method = (BoundMethod*) JSObjectGetPrivate(js_function);
		if (method == NULL)
			return JSValueMakeUndefined(js_context);

		ValueList args;
		for (size_t i = 0; i < num_args; i++) {
			Value* arg_val = KJSUtil::ToKrollValue(js_args[i], js_context, js_this);
			args.push_back(arg_val);
		}

		JSValueRef js_val = NULL;
		try
		{
			Value *ti_val = method->Call(args);
			ScopedDereferencer s(ti_val);
			js_val = KJSUtil::ToJSValue(ti_val, js_context);
		}
		catch (Value* exception)
		{
			ScopedDereferencer s(exception);
			*js_exception = KJSUtil::ToJSValue(exception, js_context);
			js_val = NULL;
		}

		for (size_t i = 0; i < num_args; i++) {
			KR_DECREF(args[i]);
		}

		return js_val;

	}

}
