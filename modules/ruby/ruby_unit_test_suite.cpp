/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"

namespace kroll
{
	void RubyUnitTestSuite::Run(Host *host)
	{
		// ints
		{
			Value *value = new Value(1);
			VALUE rubyValue = RubyUtils::ToValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT(1 == RubyUtils::ToInt(rubyValue));
			KR_DECREF(value);
		}
		
		// doubles
		{
			Value *value = new Value(1.0);
			VALUE rubyValue = RubyUtils::ToValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT(1.0 == RubyUtils::ToDouble(rubyValue));
			KR_DECREF(value);
		}
		
		// boolean TRUE
		{
			Value *value = new Value(true);
			VALUE rubyValue = RubyUtils::ToValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT(true == RubyUtils::ToBool(rubyValue));
			KR_DECREF(value);
		}
		
		// boolean FALSE
		{
			Value *value = new Value(false);
			VALUE rubyValue = RubyUtils::ToValue(value);
			KR_ASSERT(!rubyValue);
			KR_ASSERT(false == RubyUtils::ToBool(rubyValue));
			KR_DECREF(value);
		}

		// strings
		{
			std::string s("hello");
			Value *value = new Value(s);
			VALUE rubyValue = RubyUtils::ToValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT_STR(RubyUtils::ToString(rubyValue).c_str(),"hello");
			KR_DECREF(value);
		}
		
		// bound object
		{
			BoundObject *value = new StaticBoundObject();
			Value* blah = new Value("bar");
			value->Set("foo",blah);
			VALUE rubyValue = RubyUtils::Create(value);
			KR_ASSERT(rubyValue);
			VALUE result = rb_iv_get(rubyValue, "@bar");
			KR_ASSERT(result);
			KR_ASSERT(TYPE(result)==T_STRING);
//			KR_ASSERT_STR(RubyUtils::ToString(result).c_str(),"foo");
			KR_DECREF(blah);
			KR_DECREF(value);
		}
		
	}
}
