/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ruby_module.h"

namespace kroll {
	KRubyMethod::KRubyMethod(VALUE method) :
		method(method),
		arg(Qnil),
		object(new KRubyObject(method))
	{
		rb_gc_register_address(&method);
	}

	KRubyMethod::KRubyMethod(VALUE method, VALUE arg) :
		method(method),
		arg(arg),
		object(new KRubyObject(method))
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

	SharedValue KRubyMethod::Call(const ValueList& args)
	{
		// Bloody hell, Ruby will segfault if we try to pass a number
		// of args to a method that is greater than its arity
		int arity = NUM2INT(rb_apply(method, rb_intern("arity"), rb_ary_new()));
		VALUE ruby_args = rb_ary_new();

		if (this->arg != Qnil)
			rb_ary_push(ruby_args, this->arg);

		for (size_t i = 0; i < args.size() && (int) i < arity; i++) {
			rb_ary_push(ruby_args, RubyUtils::ToRubyValue(args[i]));
		}

		int error;
		VALUE do_call_args = rb_ary_new();
		rb_ary_push(do_call_args, method);
		rb_ary_push(do_call_args, ruby_args);
		VALUE result = rb_protect(do_call, do_call_args, &error);

		if (error != 0)
		{
			ValueException e = RubyUtils::GetException();
			SharedString ss = e.DisplayString();
			std::cout << "Error: " << *ss << std::endl;
			throw e;
		}

		return RubyUtils::ToKrollValue(result);
	}

	void KRubyMethod::Set(const char *name, SharedValue value)
	{
		object->Set(name, value);
	}

	SharedValue KRubyMethod::Get(const char *name)
	{
		return object->Get(name);
	}

	SharedStringList KRubyMethod::GetPropertyNames()
	{
		return object->GetPropertyNames();
	}

	SharedString KRubyMethod::DisplayString(int levels)
	{
		return this->object->DisplayString(levels);
	}

	VALUE KRubyMethod::ToRuby()
	{
		return this->object->ToRuby();
	}
}
