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

namespace kroll
{
	VALUE RubyUtils::KObjectClass = Qnil;
	VALUE RubyUtils::KMethodClass = Qnil;
	VALUE RubyUtils::KListClass = Qnil;

	bool RubyUtils::KindOf(VALUE value, VALUE klass)
	{
		return rb_obj_is_kind_of(value, klass) == Qtrue;
	}

	KValueRef RubyUtils::ToKrollValue(VALUE value)
	{
		KValueRef kvalue = Value::Undefined;

		int t = TYPE(value);
		if (T_NIL == t)
		{
			kvalue = Value::Null;
		}
		else if (T_FIXNUM == t || T_BIGNUM == t || T_FLOAT == t)
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
			KObjectRef kobj = new KRubyObject(value);
			kvalue = Value::NewObject(kobj);
		}
		else if (T_STRUCT == t)
		{
			KObjectRef kobj = new KRubyObject(value);
			kvalue = Value::NewObject(kobj);
		}
		else if (T_HASH == t)
		{
			KObjectRef kobj = new KRubyHash(value);
			kvalue = Value::NewObject(kobj);
		}
		else if (T_ARRAY == t)
		{
			KListRef klist = new KRubyList(value);
			kvalue = Value::NewList(klist);
		}
		else if (T_DATA == t && KObjectClass != Qnil && KindOf(value, KObjectClass))
		{
			KValueRef* kval = NULL;
			Data_Get_Struct(value, KValueRef, kval);
			kvalue = Value::NewObject((*kval)->ToObject());
		}
		else if (T_DATA == t && KMethodClass != Qnil && KindOf(value, KMethodClass))
		{
			KValueRef* kval = NULL;
			Data_Get_Struct(value, KValueRef, kval);
			kvalue = Value::NewMethod((*kval)->ToMethod());
		}
		else if (T_DATA == t && KListClass != Qnil && KindOf(value, KListClass))
		{
			KValueRef* kval = NULL;
			Data_Get_Struct(value, KValueRef, kval);
			kvalue = Value::NewList((*kval)->ToList());
		}
		else if (T_DATA == t && KindOf(value, rb_cMethod))
		{
			KMethodRef method = new KRubyMethod(value);
			return Value::NewMethod(method);
		}
		else if (T_DATA == t && KindOf(value, rb_cProc))
		{
			KMethodRef method = new KRubyMethod(value);
			return Value::NewMethod(method);
		}
		else if (T_DATA == t)
		{
			KObjectRef object = new KRubyObject(value);
			return Value::NewObject(object);
		}

		return kvalue;
	}

	VALUE RubyUtils::ToRubyValue(KValueRef value)
	{
		if (value->IsNull() || value->IsUndefined())
		{
			return Qnil;
		}
		if (value->IsBool())
		{
			return value->ToBool() ? Qtrue : Qfalse;
		}
		else if (value->IsNumber())
		{
			return rb_float_new(value->ToNumber());
		}
		else if (value->IsString())
		{
			return rb_str_new2(value->ToString());
		}
		else if (value->IsObject())
		{
			AutoPtr<KRubyObject> ro = value->ToObject().cast<KRubyObject>();
			if (!ro.isNull())
				return ro->ToRuby();

			AutoPtr<KRubyHash> rh = value->ToObject().cast<KRubyHash>();
			if (!rh.isNull())
				return rh->ToRuby();

			return RubyUtils::KObjectToRubyValue(value);
		}
		else if (value->IsMethod())
		{
			AutoPtr<KRubyMethod> rm = value->ToMethod().cast<KRubyMethod>();
			if (!rm.isNull())
				return rm->ToRuby();
			else
				return RubyUtils::KMethodToRubyValue(value);
		}
		else if (value->IsList())
		{
			AutoPtr<KRubyList> rl = value->ToList().cast<KRubyList>();
			if (!rl.isNull())
				return rl->ToRuby();
			else
				return RubyUtils::KListToRubyValue(value);
		}
		return Qnil;
	}

	static VALUE RubyKObjectMethods(VALUE self)
	{
		KValueRef* value = NULL;
		Data_Get_Struct(self, KValueRef, value);
		KObjectRef object = (*value)->ToObject();

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

	VALUE RubyUtils::GenericKMethodCall(KMethodRef method, VALUE args)
	{
		ValueList kargs;
		for (int i = 0; i < RARRAY_LEN(args); i++)
		{
			VALUE rarg = rb_ary_entry(args, i);
			KValueRef arg = RubyUtils::ToKrollValue(rarg);
			Value::Unwrap(arg);
			kargs.push_back(arg);
		}

		try
		{
			KValueRef result = method->Call(kargs);
			return RubyUtils::ToRubyValue(result);
		}
		catch (ValueException& e)
		{
			// TODO: Eventually wrap these up in a special exception
			// class so that we can unwrap them into ValueExceptions again
			SharedString ss = e.DisplayString();
			rb_raise(rb_eStandardError, ss->c_str());
			return Qnil;
		}
	}

	// A :method method for pulling methods off of KObjects in Ruby
	static VALUE RubyKObjectMethod(int argc, VALUE *argv, VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KObjectRef object = (*dval)->ToObject();

		// TODO: We should raise an exception instead
		if (object.isNull())
			return Qnil;
		if (argc < 1)
			return Qnil;

		VALUE meth_name = argv[0];
		const char* name = rb_id2name(SYM2ID(meth_name));
		KValueRef v = object->Get(name);
		if (v->IsMethod())
		{
			return RubyUtils::ToRubyValue(v);
		}
		else
		{
			return Qnil;
		}
	}

	// A :method_missing method for finding KObject properties in Ruby
	static VALUE RubyKObjectMethodMissing(int argc, VALUE *argv, VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KObjectRef object = (*dval)->ToObject();

		// TODO: We should raise an exception instead
		if (object.isNull())
			return Qnil;

		// This is the same error that ruby throws
		if (argc == 0 || !SYMBOL_P(argv[0]))
		{
			rb_raise(rb_eArgError, "no id given");
		}

		// We need to determine the method that was invoked:
		// store the method name and arguments in separate variables
		VALUE r_name, args;
		rb_scan_args(argc, argv, "1*", &r_name, &args);
		const char* name = rb_id2name(SYM2ID(r_name));

		// Check if this is an assignment
		KValueRef value = object->Get(name);
		if (name[strlen(name) - 1] == '=' && argc > 1)
		{
			char* mod_name = strdup(name);
			mod_name[strlen(mod_name) - 1] = '\0';
			value = RubyUtils::ToKrollValue(argv[1]);
			object->Set(mod_name, value);
			free(mod_name);
			return argv[1];
		}
		else if (value->IsUndefined()) // raise a method missing error
		{
			VALUE selfString = rb_obj_as_string(self);
			rb_raise(rb_eNoMethodError, "undefined method `%s' for %s",
				name, StringValueCStr(selfString));
		}
		else if (value->IsMethod()) // actually call a method
		{
			return RubyUtils::GenericKMethodCall(value->ToMethod(), args);
		}
		else // Plain old access
		{
			return RubyUtils::ToRubyValue(value);
		}
	}

	// A :responds_to? method for finding KObject properties in Ruby
	static VALUE RubyKObjectRespondTo(int argc, VALUE *argv, VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KObjectRef object = (*dval)->ToObject();
		VALUE mid, priv; // We ignore the priv argument

		rb_scan_args(argc, argv, "11", &mid, &priv);
		const char* name = rb_id2name(rb_to_id(mid));
		KValueRef value = object->Get(name);
		return value->IsUndefined() ? Qfalse : Qtrue;
	}

	static void RubyKObjectFree(void *p)
	{
		KValueRef* kval = static_cast<KValueRef*>(p);
		delete kval;
	}

	static VALUE RubyKMethodCall(VALUE self, VALUE args)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KMethodRef method = (*dval)->ToMethod();

		// TODO: We should raise an exception instead
		if (method.isNull())
			return Qnil;

		return RubyUtils::GenericKMethodCall(method, args);
	}

	VALUE RubyUtils::KObjectToRubyValue(KValueRef obj)
	{
		// Lazily initialize the KObject wrapper class
		if (KObjectClass == Qnil)
		{
			KObjectClass = rb_define_class("RubyKObject", rb_cObject);
			rb_define_method(KObjectClass, "method_missing",
				RUBY_METHOD_FUNC(RubyKObjectMethodMissing), -1);
			rb_define_method(KObjectClass, "method",
				RUBY_METHOD_FUNC(RubyKObjectMethod), -1);
			rb_define_method(KObjectClass, "methods",
				RUBY_METHOD_FUNC(RubyKObjectMethods), 0);
			rb_define_method(KObjectClass, "respond_to?",
				RUBY_METHOD_FUNC(RubyKObjectRespondTo), -1);
		}

		VALUE wrapper = Data_Wrap_Struct(KObjectClass, 0, RubyKObjectFree, new KValueRef(obj));
		rb_obj_call_init(wrapper, 0, 0);
		return wrapper;
	}

	VALUE RubyUtils::KMethodToRubyValue(KValueRef obj)
	{
		// Lazily initialize the KMethod wrapper class
		if (KMethodClass == Qnil)
		{
			KMethodClass = rb_define_class("RubyKMethod", rb_cObject);
			rb_define_method(KMethodClass, "method_missing",
				RUBY_METHOD_FUNC(RubyKObjectMethodMissing), -1);
			rb_define_method(KMethodClass, "method",
				RUBY_METHOD_FUNC(RubyKObjectMethod), -1);
			rb_define_method(KMethodClass, "methods",
				RUBY_METHOD_FUNC(RubyKObjectMethods), 0);
			rb_define_method(KMethodClass, "respond_to?",
				RUBY_METHOD_FUNC(RubyKObjectRespondTo), -1);
			rb_define_method(KMethodClass, "call",
				RUBY_METHOD_FUNC(RubyKMethodCall), -2);
		}

		VALUE wrapper = Data_Wrap_Struct(KMethodClass, 0, RubyKObjectFree, new KValueRef(obj));
		rb_obj_call_init(wrapper, 0, 0);
		return wrapper;
	}

	static VALUE RubyKListGetElt(int argc, VALUE *argv, VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KListRef list = (*dval)->ToList();

		// TODO: We should raise an exception instead
		if (list.isNull() || argc < 1)
			return Qnil;

		int idx = -1;
		if (TYPE(argv[0]) != T_FIXNUM || ((idx = NUM2INT(argv[0])) < 0))
			return Qnil;

		if (idx >= 0 && idx < (int) list->Size())
		{
			KValueRef v = list->At(idx);
			return RubyUtils::ToRubyValue(v);
		}
		else
		{
			return Qnil;
		}
	}

	static VALUE RubyKListSetElt(int argc, VALUE *argv, VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KListRef klist = (*dval)->ToList();

		// TODO: We should raise an exception instead
		if (klist.isNull() || argc < 2)
			return Qnil;
		// TODO: Maybe we should raise an exception instead
		if (TYPE(argv[0]) != T_FIXNUM)
			return Qnil;

		int idx = NUM2INT(argv[0]);
		if (idx < 0)
			return Qnil;

		KValueRef value = RubyUtils::ToKrollValue(argv[1]);
		klist->SetAt(idx, value);

		return argv[1];
	}

	static VALUE RubyKListLength(int argc, VALUE *argv, VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KListRef klist = (*dval)->ToList();

		// TODO: We should raise an exception instead
		if (klist.isNull())
			return Qnil;

		if (argc > 0)
		{
			rb_raise(rb_eNoMethodError, "wrong number of arguments (%d for 0)", argc);
			return Qnil;
		}
		else
		{
			return INT2NUM(klist->Size());
		}
	}

	static VALUE RubyKListEach(VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KListRef list = (*dval)->ToList();

		if (list.isNull() || !rb_block_given_p())
			return Qnil;

		for (unsigned int i = 0; i < list->Size(); i++)
			rb_yield(RubyUtils::ToRubyValue(list->At(i)));

		return self;
	}

	static VALUE RubyKListCollect(VALUE self)
	{
		KValueRef* dval = NULL;
		Data_Get_Struct(self, KValueRef, dval);
		KListRef list = (*dval)->ToList();

		if (list.isNull() || !rb_block_given_p())
			return Qnil;

		VALUE resultArray = rb_ary_new();
		for (unsigned int i = 0; i < list->Size(); i++)
			rb_ary_push(resultArray, rb_yield(RubyUtils::ToRubyValue(list->At(i))));

		return resultArray;
	}

	VALUE RubyUtils::KListToRubyValue(KValueRef obj)
	{
		// Lazily initialize the KMethod wrapper class
		if (KListClass == Qnil)
		{
			KListClass = rb_define_class("RubyKList", rb_cObject);
			rb_define_method(KListClass, "method_missing",
				RUBY_METHOD_FUNC(RubyKObjectMethodMissing), -1);
			rb_define_method(KListClass, "method",
				RUBY_METHOD_FUNC(RubyKObjectMethod), -1);
			rb_define_method(KListClass, "methods",
				RUBY_METHOD_FUNC(RubyKObjectMethods), 0);
			rb_define_method(KListClass, "respond_to?",
				RUBY_METHOD_FUNC(RubyKObjectRespondTo), -1);
			rb_define_method(KListClass, "[]",
				RUBY_METHOD_FUNC(RubyKListGetElt), -1);
			rb_define_method(KListClass, "[]=",
				RUBY_METHOD_FUNC(RubyKListSetElt), -1);
			rb_define_method(KListClass, "length",
				RUBY_METHOD_FUNC(RubyKListLength), -1);
			rb_define_method(KListClass, "each",
				RUBY_METHOD_FUNC(RubyKListEach), 0);
			rb_define_method(KListClass, "collect",
				RUBY_METHOD_FUNC(RubyKListCollect), 0);
			rb_define_method(KListClass, "map",
				RUBY_METHOD_FUNC(RubyKListCollect), 0);
		}

		VALUE wrapper = Data_Wrap_Struct(KListClass, 0, RubyKObjectFree, new KValueRef(obj));
		rb_obj_call_init(wrapper, 0, 0);
		return wrapper;
	}

	ValueException RubyUtils::GetException()
	{
		VALUE e = rb_gv_get("$!");
		KValueRef v = RubyUtils::ToKrollValue(e);
		return ValueException(v);
	}
}
