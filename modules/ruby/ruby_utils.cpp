/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#include "k_ruby_object.h"
#include "k_ruby_method.h"
#include "ruby_method_missing.h"

namespace kroll
{
	VALUE RubyUtils::kobj_class = Qnil;
	VALUE RubyUtils::kmethod_class = Qnil;
	VALUE RubyUtils::klist_class = Qnil;

	const char* RubyUtils::ToString(VALUE value)
	{
		if (TYPE(value)==T_STRING)
		{
			const char *result = StringValueCStr(value);
			return result;
		}
		return NULL;
	}

	SharedValue RubyUtils::ToKrollValue(VALUE value)
	{
		SharedValue kvalue = Value::Undefined;

		int t = TYPE(value);
		VALUE c = rb_obj_class(value);
		if (T_NIL == t)
		{
			kvalue == Value::Null;
		}
		else if (T_FIXNUM == t)
		{
			kvalue = Value::NewInt(NUM2INT(value));
		}
		else if (T_FIXNUM == t)
		{
			kvalue = Value::NewDouble(NUM2DBL(value));
		}
		else if (T_TRUE == t)
		{
			kvalue = Value::NewBool(true);
		}
		else if (T_FALSE == t)
		{
			kvalue = Value::NewBool(false);
		}
		else if (T_STRING == t)
		{
			char* ptr = StringValuePtr(value);
			kvalue = Value::NewString(ptr);
		}
		else if (T_OBJECT == t)
		{
			SharedKObject kobj = new KRubyObject(value);
			kvalue = Value::NewObject(kobj);
		}
		else if (T_ARRAY == t)
		{
			SharedKList klist = new KRubyList(value);
			kvalue = Value::NewList(klist);
		}
		else if (T_DATA == t && kobj_class != Qnil && c == kobj_class)
		{
			SharedValue* kval = NULL;
			Data_Get_Struct(value, SharedValue, kval);
			kvalue = Value::NewObject((*kval)->ToObject());
		}
		else if (T_DATA == t && kmethod_class != Qnil && c == kmethod_class)
		{
			SharedValue* kval = NULL;
			Data_Get_Struct(value, SharedValue, kval);
			kvalue = Value::NewMethod((*kval)->ToMethod());
		}
		else if (T_DATA == t && klist_class != Qnil && c == klist_class)
		{
			SharedValue* kval = NULL;
			Data_Get_Struct(value, SharedValue, kval);
			kvalue = Value::NewList((*kval)->ToList());
		}
		else if (T_DATA == t && c == rb_cMethod)
		{
			SharedKMethod method = new KRubyMethod(value);
			return Value::NewMethod(method);
		}
		else if (T_DATA == t)
		{
			SharedKObject object = new KRubyObject(value);
			return Value::NewObject(object);
		}

		return kvalue;
	}

	VALUE RubyUtils::ToRubyValue(SharedValue value)
	{
		if (value->IsNull() || value->IsUndefined())
		{
			return Qnil;
		}
		if (value->IsBool())
		{
			return value->ToBool() ? Qtrue : Qfalse;
		}
		else if (value->IsInt())
		{
			return INT2NUM(value->ToInt());
		}
		else if (value->IsString())
		{
			return rb_str_new2(value->ToString());
		}
		else if (value->IsDouble())
		{
			return rb_float_new(value->ToDouble());
		}
		else if (value->IsObject())
		{
			return RubyUtils::KObjectToRubyValue(value);
		}
		else if (value->IsMethod())
		{
			return RubyUtils::KMethodToRubyValue(value);
		}
		else if (value->IsList())
		{
			return RubyUtils::KListToRubyValue(value);
		}
		return Qnil;
	}

	// Override method_defined? to support reflection of
	// methods to determine if they are defined on an object
	static VALUE ruby_kobject_method_defined(int argc, VALUE *argv, VALUE self)
	{
		SharedValue* value = NULL;
		Data_Get_Struct(self, SharedValue, value);
		SharedKObject object = (*value)->ToObject();

		// TODO: We should raise an exception instead
		if (object.isNull() || argc < 1)
		{
			return Qfalse;
		}
		else
		{
			const char* method_name = rb_id2name(SYM2ID(argv[0]));
			SharedValue methval = object->Get(method_name);
			return methval->IsMethod() ? Qtrue : Qfalse;
		}
	}

	static VALUE ruby_kobject_methods(VALUE self)
	{
		SharedValue* value = NULL;
		Data_Get_Struct(self, SharedValue, value);
		SharedKObject object = (*value)->ToObject();

		VALUE* args = NULL;
		VALUE methods = rb_call_super(0, args);

		SharedStringList props = object->GetPropertyNames();
		for (unsigned int i = 0; i < props->size(); i++)
		{
			SharedString prop_name = props->at(i);
			rb_ary_push(methods, rb_str_new2(prop_name->c_str()));
		}
		return methods;
	}

	VALUE RubyUtils::GenericKMethodCall(SharedKMethod method, VALUE args)
	{
		ValueList kargs;
		for (int i = 0; i < RARRAY(args)->len; i++)
		{
			VALUE rarg = rb_ary_entry(args, i);
			SharedValue arg = RubyUtils::ToKrollValue(rarg);
			kargs.push_back(arg);
		}

		try
		{
			SharedValue result = method->Call(kargs);
			return RubyUtils::ToRubyValue(result);
		}
		catch (ValueException& e)
		{
			// TODO: Eventually wrap these up in a special exception
			// class so that we can unwrap them into ValueExceptions again
			SharedString ss = e.DisplayString();
			VALUE rex = rb_str_new2(ss->c_str());
			rb_raise(rex, "KrollException");
			return Qnil;
		}
	}

	// A method_missing magic function that's called
	// for any access to our object (essentially)
	static VALUE ruby_kobject_method_missing(int argc, VALUE *argv, VALUE self)
	{
		SharedValue* dval = NULL;
		Data_Get_Struct(self, SharedValue, dval);
		SharedKObject object = (*dval)->ToObject();

		// TODO: We should raise an exception instead
		if (object.isNull())
			return Qnil;

		// We need to determine the method that was invoked:
		// store the method name and arguments in separate variables
		VALUE r_name, args;
		rb_scan_args(argc, argv, "1*", &r_name, &args);
		const char* name = rb_id2name(SYM2ID(r_name));

		// Check if this is an assignment
		SharedValue value = object->Get(name);
		if (name[strlen(name) - 1] == '=' && argc > 1)
		{
			char* mod_name = strdup(name);
			mod_name[strlen(mod_name) - 1] = '\0';
			value = RubyUtils::ToKrollValue(argv[1]);
			object->Set(mod_name, value);
			free(mod_name);
			return argv[1];
		}
		else if (value->IsMethod())
		{
			return RubyUtils::GenericKMethodCall(value->ToMethod(), args);
		}
		else // Plain old access
		{
			return RubyUtils::ToRubyValue(value);
		}
	}

	static void ruby_kobject_free(void *p)
	{
		SharedValue* kval = static_cast<SharedValue*>(p);
		delete kval;
	}

	static VALUE ruby_kmethod_call(VALUE self, VALUE args)
	{
		SharedValue* dval = NULL;
		Data_Get_Struct(self, SharedValue, dval);
		SharedKMethod method = (*dval)->ToMethod();

		// TODO: We should raise an exception instead
		if (method.isNull())
			return Qnil;

		return RubyUtils::GenericKMethodCall(method, args);
	}

	VALUE RubyUtils::KObjectToRubyValue(SharedValue obj)
	{
		// Lazily initialize the KObject wrapper class
		if (kobj_class == Qnil)
		{
			kobj_class = rb_define_class("RubyKObject", rb_cObject);
			rb_define_method(kobj_class, "method_missing",
				RUBY_METHOD_FUNC(ruby_kobject_method_missing), -1);
			rb_define_method(kobj_class, "method_defined?",
				RUBY_METHOD_FUNC(ruby_kobject_method_defined), -1);
			rb_define_method(kobj_class, "methods",
				RUBY_METHOD_FUNC(ruby_kobject_methods), 0);
		}

		VALUE wrapper = Data_Wrap_Struct(kobj_class, 0, ruby_kobject_free, new SharedValue(obj));
		rb_obj_call_init(wrapper, 0, 0);
		return wrapper;
	}

	VALUE RubyUtils::KMethodToRubyValue(SharedValue obj)
	{
		// Lazily initialize the KMethod wrapper class
		if (kmethod_class == Qnil)
		{
			kmethod_class = rb_define_class("RubyKMethod", rb_cObject);
			rb_define_method(kmethod_class, "method_missing",
				RUBY_METHOD_FUNC(ruby_kobject_method_missing), -1);
			rb_define_method(kmethod_class, "method_defined?",
				RUBY_METHOD_FUNC(ruby_kobject_method_defined), -1);
			rb_define_method(kmethod_class, "methods",
				RUBY_METHOD_FUNC(ruby_kobject_methods), 0);
			rb_define_method(kmethod_class, "call",
				RUBY_METHOD_FUNC(ruby_kmethod_call), -2);
		}

		VALUE wrapper = Data_Wrap_Struct(kmethod_class, 0, ruby_kobject_free, new SharedValue(obj));
		rb_obj_call_init(wrapper, 0, 0);
		return wrapper;
	}

	static VALUE ruby_klist_getelt(int argc, VALUE *argv, VALUE self)
	{
		SharedValue* dval = NULL;
		Data_Get_Struct(self, SharedValue, dval);
		SharedKList list = (*dval)->ToList();

		// TODO: We should raise an exception instead
		if (list.isNull() || argc < 1)
			return Qnil;

		int idx = -1;
		if (TYPE(argv[0]) != T_FIXNUM || ((idx = NUM2INT(argv[0])) < 0))
			return Qnil;

		if (idx >= 0 && idx < (int) list->Size())
		{
			SharedValue v = list->At(idx);
			return RubyUtils::ToRubyValue(v);
		}
		else
		{
			return Qnil;
		}
	}

	static VALUE ruby_klist_setelt(int argc, VALUE *argv, VALUE self)
	{
		SharedValue* dval = NULL;
		Data_Get_Struct(self, SharedValue, dval);
		SharedKList klist = (*dval)->ToList();

		// TODO: We should raise an exception instead
		if (klist.isNull() || argc < 2)
			return Qnil;
		// TODO: Maybe we should raise an exception instead
		if (TYPE(argv[0]) != T_FIXNUM)
			return Qnil;

		int idx = NUM2INT(argv[0]);
		if (idx < 0)
			return Qnil;

		SharedValue value = RubyUtils::ToKrollValue(argv[1]);
		klist->SetAt(idx, value);

		return argv[1];
	}

	static VALUE ruby_klist_each(VALUE self)
	{
		SharedValue* dval = NULL;
		Data_Get_Struct(self, SharedValue, dval);
		SharedKList list = (*dval)->ToList();

		if (list.isNull() || !rb_block_given_p())
			return Qnil;

		for (unsigned int i = 0; i < list->Size(); i++)
		{
			VALUE v = RubyUtils::ToRubyValue(list->At(i));
			VALUE args = rb_ary_new();
			rb_ary_push(args, v);
			rb_yield(args);
		}
		return self;
	}

	VALUE RubyUtils::KListToRubyValue(SharedValue obj)
	{
		// Lazily initialize the KMethod wrapper class
		if (klist_class == Qnil)
		{
			klist_class = rb_define_class("RubyKMethod", rb_cObject);
			rb_define_method(klist_class, "method_missing",
				RUBY_METHOD_FUNC(ruby_kobject_method_missing), -1);
			rb_define_method(klist_class, "method_defined?",
				RUBY_METHOD_FUNC(ruby_kobject_method_defined), -1);
			rb_define_method(klist_class, "methods",
				RUBY_METHOD_FUNC(ruby_kobject_methods), 0);
			rb_define_method(klist_class, "[]",
				RUBY_METHOD_FUNC(ruby_klist_getelt), -1);
			rb_define_method(klist_class, "[]=",
				RUBY_METHOD_FUNC(ruby_klist_setelt), -1);
			rb_define_method(klist_class, "each",
				RUBY_METHOD_FUNC(ruby_klist_each), 0);
		}

		VALUE wrapper = Data_Wrap_Struct(klist_class, 0, ruby_kobject_free, new SharedValue(obj));
		rb_obj_call_init(wrapper, 0, 0);
		return wrapper;
	}

	ValueException RubyUtils::GetException()
	{
		VALUE e = rb_gv_get("$!");
		SharedValue v = RubyUtils::ToKrollValue(e);
		return ValueException(v);
	}
}
