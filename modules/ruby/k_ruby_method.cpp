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

	SharedValue KRubyMethod::Call(const ValueList& args)
	{
		VALUE* ruby_args = (VALUE*)malloc(sizeof(VALUE)*args.size()+1);
		for (size_t i = 0; i < args.size(); i++) {
			ruby_args[i] = RubyUtils::ToRubyValue(args[i]);
		}

		VALUE result = rb_funcall2(method, rb_intern("call"), args.size(), ruby_args);

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
