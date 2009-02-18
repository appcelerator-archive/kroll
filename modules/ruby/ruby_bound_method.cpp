/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ruby_bound_method.h"
#include "ruby_bound_object.h"

namespace kroll {
	RubyBoundMethod::RubyBoundMethod(VALUE method) : method(method), delegate(new RubyBoundObject(method))
	{
		rb_gc_register_address(&method);
	}

	RubyBoundMethod::~RubyBoundMethod() {
		rb_gc_unregister_address(&method);
	}

	SharedValue RubyBoundMethod::Call(const ValueList& args)
	{
		VALUE* ruby_args = (VALUE*)malloc(sizeof(VALUE)*args.size()+1);
		for (int i = 0; i < args.size(); i++) {
			ruby_args[i] = RubyUtils::ToRubyValue(args[i]);
		}

		VALUE result = rb_funcall2(method, rb_intern("call"), args.size(), ruby_args);

		return RubyUtils::ToKrollValue(result);
	}

	void RubyBoundMethod::Set(const char *name, SharedValue value)
	{
		delegate->Set(name, value);
	}

	SharedValue RubyBoundMethod::Get(const char *name)
	{
		return delegate->Get(name);
	}

	SharedStringList RubyBoundMethod::GetPropertyNames()
	{
		return delegate->GetPropertyNames();
	}
}
