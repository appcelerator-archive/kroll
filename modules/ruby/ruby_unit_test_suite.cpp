/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module.h"

namespace kroll
{
	class TestClass : public StaticBoundObject
	{
	public:
		void TestMethod(const ValueList& args, SharedValue result)
		{
			result->SetValue(args.at(0));
		}
		TestClass()
		{
			this->SetMethod("test",&TestClass::TestMethod);
		}
		virtual ~TestClass() {}
	};

	void RubyUnitTestSuite::Run(Host *host)
	{
		// ints
		{
			SharedValue value = Value::NewInt(1);
			VALUE rubyValue = RubyUtils::ToRubyValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT(1 == RubyUtils::ToInt(rubyValue));
			//KR_DECREF(value);
		}

		// doubles
		{
			SharedValue value = Value::NewDouble(1.0);
			VALUE rubyValue = RubyUtils::ToRubyValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT(1.0 == RubyUtils::ToDouble(rubyValue));
			//KR_DECREF(value);
		}

		// boolean TRUE
		{
			SharedValue value = Value::NewBool(true);
			VALUE rubyValue = RubyUtils::ToRubyValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT(true == RubyUtils::ToBool(rubyValue));
			//KR_DECREF(value);
		}

		// boolean FALSE
		{
			SharedValue value = Value::NewBool(false);
			VALUE rubyValue = RubyUtils::ToRubyValue(value);
			KR_ASSERT(!rubyValue);
			KR_ASSERT(false == RubyUtils::ToBool(rubyValue));
			//KR_DECREF(value);
		}

		// strings
		{
			std::string s("hello");
			SharedValue value = Value::NewString(s);
			VALUE rubyValue = RubyUtils::ToRubyValue(value);
			KR_ASSERT(rubyValue);
			KR_ASSERT_STR(RubyUtils::ToString(rubyValue),"hello");
			//KR_DECREF(value);
		}

		// bound object
		{
			SharedPtr<StaticBoundObject> value = new StaticBoundObject();
			SharedValue blah = Value::NewString("bar");
			value->Set("foo",blah);
			SharedPtr<KObject> o = value;
			VALUE rubyValue = RubyUtils::Create(o);
			KR_ASSERT(rubyValue);

			// simply define as global so we can use it
			rb_define_variable("$jeff",&rubyValue);

			// access like a getter
			VALUE result = rb_eval_string("$jeff.foo");
			KR_ASSERT(result);
			KR_ASSERT(TYPE(result)==T_STRING);
			KR_ASSERT_STR(RubyUtils::ToString(result),"bar");

			// access like a setter
			rb_eval_string("$jeff.foo='blah'");
			result = rb_eval_string("$jeff.foo");
			KR_ASSERT(result);
			KR_ASSERT(TYPE(result)==T_STRING);
			KR_ASSERT_STR(RubyUtils::ToString(result),"blah");

			// dynamically attach a new object with a method
			SharedPtr<KObject> testClass = new TestClass;
			SharedValue testValue = Value::NewObject(testClass);
			value->Set("yoyo",testValue);

			// invoke the method traditionally
			result = rb_eval_string("$jeff.yoyo.test('a')");
			KR_ASSERT(result);
			KR_ASSERT(TYPE(result)==T_STRING);
			KR_ASSERT_STR(RubyUtils::ToString(result),"a");

			// invoke the method ruby-style
			result = rb_eval_string("$jeff.yoyo.test 'b'");
			KR_ASSERT(result);
			KR_ASSERT(TYPE(result)==T_STRING);
			KR_ASSERT_STR(RubyUtils::ToString(result),"b");

			// invoke undefined method and make sure it's NULL
			result = rb_eval_string("$jeff.yoyo.blah");
			KR_ASSERT(result);
			KR_ASSERT(TYPE(result)==T_NIL);
			SharedValue vr = RubyUtils::ToKrollValue(result);
			KR_ASSERT(vr->IsNull());

			// test to see if our method is defined
			result = rb_eval_string("$jeff.yoyo.method_defined? :test");
			KR_ASSERT(result);
			KR_ASSERT(RubyUtils::ToBool(result));

			// test to see if a non-existant method is not defined
			result = rb_eval_string("$jeff.yoyo.method_defined? :xbar");
			KR_ASSERT(!result);
			KR_ASSERT(!RubyUtils::ToBool(result));

			/*KR_DECREF(vr);
			KR_DECREF(testValue);
			KR_DECREF(testClass);
			KR_DECREF(blah);
			KR_DECREF(value);*/
		}

	}
}
