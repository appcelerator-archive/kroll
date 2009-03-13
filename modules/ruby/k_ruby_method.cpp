/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "k_ruby_method.h"
#include "k_ruby_object.h"

namespace kroll {
	KRubyMethod::KRubyMethod(VALUE method) :
		method(method),
		delegate(new KRubyObject(method))
	{
		rb_gc_register_address(&method);
	}

	KRubyMethod::~KRubyMethod() {
		rb_gc_unregister_address(&method);
	}

	VALUE do_call(VALUE args)
	{
		VALUE method = rb_ary_shift(args);
		VALUE rargs = rb_ary_shift(args);
		return rb_apply(method, rb_intern("call"), rargs);
	}
	VALUE handle_exception(VALUE args)
	{
		ValueException e = RubyUtils::GetException(1);
		throw e;
	}
	SharedValue KRubyMethod::Call(const ValueList& args)
	{
		// Bloody hell, Ruby will segfault if we try to pass a number
		// of args to a method that is greater than its arity
		int arity = NUM2INT(rb_apply(method, rb_intern("arity"), rb_ary_new()));
		VALUE ruby_args = rb_ary_new();
		for (size_t i = 0; i < args.size() && (int) i < arity; i++) {
			rb_ary_push(ruby_args, RubyUtils::ToRubyValue(args[i]));
		}

		// TODO: Exception handling
		VALUE do_call_args = rb_ary_new();
		rb_ary_push(do_call_args, method);
		rb_ary_push(do_call_args, ruby_args);
		VALUE result = rb_rescue(
			VALUEFUNC(do_call), do_call_args,
			VALUEFUNC(handle_exception), do_call_args);
		//VALUE result = rb_apply(method, rb_intern("call"), ruby_args);
		return RubyUtils::ToKrollValue(result);
	}

	void KRubyMethod::Set(const char *name, SharedValue value)
	{
		delegate->Set(name, value);
	}

	SharedValue KRubyMethod::Get(const char *name)
	{
		return delegate->Get(name);
	}

	SharedStringList KRubyMethod::GetPropertyNames()
	{
		return delegate->GetPropertyNames();
	}
}
