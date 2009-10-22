/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"

namespace kroll
{
	KRubyObject::KRubyObject(VALUE object) :
		KObject("Ruby.KRubyObject"),
		object(object)
	{
		rb_gc_register_address(&object);
	}

	KRubyObject::~KRubyObject()
	{
		rb_gc_unregister_address(&object);
	}

	VALUE kobj_do_method_missing_call(VALUE args)
	{
		VALUE object = rb_ary_shift(args);
		return rb_apply(object, rb_intern("method_missing"), args);
	}

	void KRubyObject::Set(const char *name, KValueRef value)
	{
		VALUE ruby_value = RubyUtils::ToRubyValue(value);
		std::string setter_name = std::string(name) + "=";
		ID set_ID = rb_intern(setter_name.c_str());

		int error = 0;
		if (rb_obj_respond_to(object, set_ID, Qtrue) == Qtrue)
		{
			rb_funcall(object, set_ID, 1, ruby_value);
		}
		else
		{
			// First try calling method missing
			VALUE rargs = rb_ary_new();
			rb_ary_push(rargs, object);
			rb_ary_push(rargs, rb_str_new2(name));
			rb_ary_push(rargs, ruby_value);
			rb_protect(kobj_do_method_missing_call, rargs, &error);

			// If the exception wasn't a normal NoMethod exception actually
			// throw an exception here, because something went wrong.
			VALUE exception = rb_gv_get("$!");
			if (rb_obj_is_kind_of(exception, rb_eNoMethodError) != Qtrue
				&& rb_obj_is_kind_of(exception,rb_eNameError) == Qtrue)
			{
				KValueRef exceptionValue = RubyUtils::ToKrollValue(exception);
				ValueException e = ValueException(exceptionValue);
				throw e;
			}
		}

		// Last resort: set an instance variable
		if (error != 0)
		{
			std::string iv_name = std::string("@") + name;
			rb_iv_set(object, iv_name.c_str(), ruby_value);
		}
	}

	KValueRef KRubyObject::Get(const char *name)
	{
		std::string iv_name = std::string("@") + name;
		ID iv_ID = rb_intern(iv_name.c_str());
		ID get_ID = rb_intern(name);
		ID mm_ID = rb_intern("method_missing");

		VALUE ruby_value = Qnil;
		if (rb_obj_respond_to(object, get_ID, Qtrue) == Qtrue)
		{
			ruby_value = rb_funcall(object, rb_intern("method"), 1, ID2SYM(get_ID));
		}
		else if (rb_ivar_defined(object, iv_ID))
		{
			ruby_value = rb_ivar_get(object, iv_ID);
		}
		else if (rb_obj_respond_to(object, mm_ID, Qtrue) == Qtrue)
		{
			// If this object has a method_missing, call that and return the result,
			int error;
			VALUE rargs = rb_ary_new();
			rb_ary_push(rargs, object);
			rb_ary_push(rargs, ID2SYM(get_ID));
			ruby_value = rb_protect(kobj_do_method_missing_call, rargs, &error);

			// protect against NoMethodErrors which we don't want to propogate
			// back through Kroll, but other exceptions should be thrown.
			VALUE exception = rb_gv_get("$!");
			if (rb_obj_is_kind_of(exception, rb_eNoMethodError) == Qtrue
				|| rb_obj_is_kind_of(exception,rb_eNameError) == Qtrue)
			{
				return Value::Undefined;
			}
			else
			{
				KValueRef exceptionValue = RubyUtils::ToKrollValue(exception);
				ValueException e = ValueException(exceptionValue);
				throw e;
			}
		}

		return RubyUtils::ToKrollValue(ruby_value);
	}

	bool KRubyObject::Equals(KObjectRef other)
	{
		AutoPtr<KRubyObject> rubyOther = other.cast<KRubyObject>();
		if (rubyOther.isNull())
			return false;

		return this->ToRuby() == rubyOther->ToRuby();
	}

	SharedString KRubyObject::DisplayString(int levels)
	{
		VALUE out = rb_obj_as_string(object);
		return new std::string(StringValueCStr(out));
	}

	SharedStringList KRubyObject::GetPropertyNames()
	{
		SharedStringList names(new StringList());
		VALUE vars = rb_obj_instance_variables(object);
		for (int i = 0; i < RARRAY_LEN(vars); i++)
		{
			VALUE prop_name = rb_ary_entry(vars, i);
			std::string name(StringValueCStr(prop_name));
			if (name[0] == '@')
				name = name.substr(1);

			names->push_back(new std::string(name));
		}

		VALUE methodsSymbol = rb_intern("methods");
		if (!rb_obj_respond_to(object, methodsSymbol, Qfalse))
			return names;

		VALUE methods = rb_funcall(object, rb_intern("methods"), 0);
		for (int i = 0; i < RARRAY_LEN(methods); i++)
		{
			VALUE rmethodName = rb_ary_entry(methods, i);
			const char *methodName = StringValueCStr(rmethodName);
			names->push_back(new std::string(methodName));
		}

		return names;
	}

	VALUE KRubyObject::ToRuby()
	{
		return this->object;
	}
}
