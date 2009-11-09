/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef RUBY_TYPES_H_
#define RUBY_TYPES_H_

#include <typeinfo>
#include "ruby_module.h"

namespace kroll
{
	class RubyUtils
	{
	public:
		static KValueRef ToKrollValue(VALUE value);
		static VALUE ToRubyValue(KValueRef value);
		static VALUE KObjectToRubyValue(KValueRef value);
		static VALUE KMethodToRubyValue(KValueRef value);
		static VALUE KListToRubyValue(KValueRef value);
		static bool KindOf(VALUE value, VALUE klass);

		static ValueException GetException();
		static VALUE GenericKMethodCall(KMethodRef method, VALUE args);

	private:
		static VALUE KObjectClass;
		static VALUE KMethodClass;
		static VALUE KListClass;
		RubyUtils(){}
		~RubyUtils(){}
	};
}

#endif
