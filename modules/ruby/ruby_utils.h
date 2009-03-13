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
		static void InitializeDefaultBindings(Host *host);

		static const char* TypeToString (int type);
		static VALUE Create(SharedPtr<BoundObject> value);
		static VALUE Create(SharedPtr<BoundList> list);
		static const char * ToString(VALUE value);
		static bool ToBool(VALUE value);
		static int ToInt(VALUE value);
		static double ToDouble(VALUE value);
		static SharedValue ToKrollValue(VALUE value);
		static VALUE ToRubyValue(SharedValue value);
		static const char * ToUpper (const char *s);
		static SharedBoundMethod ToMethod(VALUE value);
		static SharedBoundObject ToObject(VALUE value);
		static SharedBoundMethod CreateMethodMissing(VALUE object, std::string& method_name);

		static ValueException GetException(int);

	private:
		static SharedPtr<BoundObject> scope;

		RubyUtils(){}
		~RubyUtils(){}
	};
}

#endif
