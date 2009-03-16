/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "api_unit_test_suite.h"
#include <cstring>

namespace kroll
{
	class TestClass : public KMethod
	{
	public:
		SharedValue Call(const ValueList& args)
		{
			SharedValue e = args.at(0);
			if (e->IsString())
			{
				this->event = e->ToString();
				if (args.size() > 1)
				{
					e = args.at(1);
					if (e->IsString())
					{
						this->message = e->ToString();
					}
				}
			}
			return Value::Undefined;
		}
		void Set(const char *name, SharedValue value)
		{

		}
		SharedValue Get(const char *name)
		{
			return NULL;
		}
		virtual SharedStringList GetPropertyNames()
		{
			return NULL;
		}

		std::string& Event() { return event; }
		std::string& Message() { return message;}
		void Reset()
		{
			event = "";
			message = "";
		}
	private:
		std::string event;
		std::string message;
	};


	void APIUnitTestSuite::Run(Host *host, SharedKObject binding)
	{
		std::string msg("hello");
		int severity = KR_LOG_DEBUG;

		SharedPtr<APIBinding> api = binding.cast<APIBinding>();

		SharedValue logMethodValue = binding->Get("log");
		SharedKMethod logMethod = logMethodValue->ToMethod();
		KR_ASSERT(!logMethod.isNull());

		ValueList args;
		SharedValue sv = Value::NewInt(severity);
		args.push_back(sv);
		SharedValue mv = Value::NewString(msg);
		args.push_back(mv);

		// invoke directly against interface
		api->Log(severity,msg);

		// invoke with ValueList
		logMethod->Call(args);

		SharedValue v = Value::NewInt(1);
		binding->Set("foo",v);
		SharedValue vr = binding->Get("foo");
		KR_ASSERT(v->ToInt() == vr->ToInt());

		// TEST retrieving the value from a namespaced string
		SharedValue foo = host->GetGlobalObject()->GetNS("API.foo");
		KR_ASSERT(foo->ToInt()==1);

		// TEST registering an event handler and then receiving it
		// once it's fired
		std::string event("kr.api.log");
		SharedKMethod sTestObject = new TestClass();
		TestClass* testObject = (TestClass*) sTestObject.get();

		int ref = api->Register(event,sTestObject);
		KR_ASSERT(ref!=0);
		SharedValue data = Value::NewString("some data here");

		api->Fire(event.c_str(),data);
		KR_ASSERT_STR(testObject->Event().c_str(),"kr.api.log");
		KR_ASSERT_STR(testObject->Message().c_str(),"some data here");

		testObject->Reset();

		// TEST unregister and then refire -- we shouldn't receive it
		api->Unregister(ref);

		api->Fire(event.c_str(),data);
		KR_ASSERT_STR(testObject->Event().c_str(),"");
		KR_ASSERT_STR(testObject->Message().c_str(),"");

		// TEST basic list implementation
		{
			SharedKList l = new StaticBoundList();
			KR_ASSERT(l->Size()==0);
			SharedValue i = Value::NewInt(1);
			l->Append(i);
			KR_ASSERT(l->Size()==1);
			SharedValue i2 = l->At(0);
			KR_ASSERT(i == i2);
			SharedValue i3 = l->Get("length");
			KR_ASSERT(i3->ToInt());
		}
	}
}
