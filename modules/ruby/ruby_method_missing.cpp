/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ruby_method_missing.h"

RubyMethodMissing::RubyMethodMissing(VALUE object, std::string& name) : object(object), method_name(name) {

}

RubyMethodMissing::~RubyMethodMissing() {
}

SharedValue RubyMethodMissing::Call(const ValueList& args)
{
	VALUE* ruby_args = (VALUE*)malloc((sizeof(VALUE)*(args.size()+1))+1);
	ruby_args[0] = rb_str_new2(method_name.c_str());
	for (size_t i = 0; i < args.size(); i++) {
		ruby_args[i] = RubyUtils::ToRubyValue(args[i-1]);
	}

	VALUE result = rb_funcall2(object, rb_intern("method_missing"), args.size(), ruby_args);

	return RubyUtils::ToKrollValue(result);
}

void RubyMethodMissing::Set(const char *name, SharedValue value)
{

}

SharedValue RubyMethodMissing::Get(const char *name)
{
	return Value::Undefined;
}

SharedStringList RubyMethodMissing::GetPropertyNames()
{
	SharedStringList list = new StringList();
	return list;
}
