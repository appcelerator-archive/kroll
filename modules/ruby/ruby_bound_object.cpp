/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ruby_bound_object.h"

namespace kroll {

	RubyBoundObject::RubyBoundObject(VALUE object) : object(object) {
		rb_gc_register_address(&object);

	}

	RubyBoundObject::~RubyBoundObject() {
		rb_gc_unregister_address(&object);
	}

	void RubyBoundObject::Set(const char *name, SharedValue value)
	{
		VALUE rubyValue = RubyUtils::ToRubyValue(value);
		std::string instance_name = "@";
		instance_name += name;

		rb_iv_set(object, instance_name.c_str(), rubyValue);
	}

	SharedValue RubyBoundObject::Get(const char *name)
	{
		std::string method_name = name;
		std::string instance_name = "@" ;
		instance_name += method_name;

		ID varID = rb_intern(instance_name.c_str());
		ID methodID = rb_intern(name);

		if (rb_ivar_defined(object, varID) == Qtrue) {
			return RubyUtils::ToKrollValue(rb_iv_get(object, instance_name.c_str()));
		}

		if (rb_funcall(object, rb_intern("respond_to?"), 1, ID2SYM(methodID)) == Qtrue)
		{
			VALUE method = rb_funcall(object, rb_intern("method"), 1, ID2SYM(methodID));
			SharedBoundMethod bound_method = RubyUtils::ToMethod(method);
			return Value::NewMethod(bound_method);
		}

		SharedBoundMethod missing = RubyUtils::CreateMethodMissing(object, method_name);
		return Value::NewMethod(missing);
	}


	SharedStringList RubyBoundObject::GetPropertyNames()
	{
		VALUE instance_variables = rb_obj_instance_variables(rb_obj_class(object));
		VALUE lengthValue = rb_funcall(instance_variables, rb_intern("length"), 0);
		int length = RubyUtils::ToInt(lengthValue);

		SharedStringList property_names(new StringList());

		for (int i = 0; i < length; i++)
		{
			VALUE property = rb_funcall(instance_variables, rb_intern("at"), 1, RubyUtils::ToInt(i));
			property_names->push_back(new std::string(RubyUtils::ToString(property)));
		}

		return property_names;
	}
}
