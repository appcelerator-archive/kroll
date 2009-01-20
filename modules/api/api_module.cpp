/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "api_module.h"
#include <string.h>
#include <iostream>
#include <vector>

namespace kroll
{
	KROLL_MODULE(APIModule)

	void APIModule::Initialize()
	{
		binding = new APIBinding(host->GetGlobalObject());
		host->GetGlobalObject()->SetObject("api", binding);
	}

	void APIModule::Destroy()
	{
		//KR_DECREF(binding);
	}

	class TestClass : public BoundMethod
	{
	public:
		SharedValue Call(const ValueList& args)
		{
			SharedValue e = args.at(0);
			if (e->IsString())
			{
				this->event = e->ToString();
				e = args.at(1);
				if (e->IsString())
				{
					this->message = e->ToString();
				}
			}
			return NULL;
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


	void APIModule::Test()
	{
		std::string msg("hello");
		int severity = KR_LOG_DEBUG;

		SharedValue logMethodValue = binding->Get("log");
		SharedPtr<BoundMethod> logMethod = logMethodValue->ToMethod();
		KR_ASSERT(!logMethod.isNull());

		ValueList args;
		SharedValue sv = Value::NewInt(severity);
		args.push_back(sv);
		SharedValue mv = Value::NewString(msg);
		args.push_back(mv);

		// invoke directly against interface
		binding->Log(severity,msg);

		// invoke with ValueList
		logMethod->Call(args);

		// invoke with varargs
		//logMethod->Call(sv,mv);

		SharedValue v = Value::NewInt(1);
		binding->Set("foo",v);
		SharedValue vr = binding->Get("foo");
		KR_ASSERT(v->ToInt() == vr->ToInt());
		//KR_DECREF(v);
		//KR_DECREF(vr);

		// TEST retrieving the value from a namespaced string
		SharedValue foo = host->GetGlobalObject()->GetNS("api.foo");
		KR_ASSERT(foo->ToInt()==1);
		//KR_DECREF(foo);

		// TEST registering an event handler and then receiving it
		// once it's fired
		std::string event("kr.api.log");
		TestClass* testObject = new TestClass;
		int ref = binding->Register(event,testObject);
		SharedValue data = Value::NewString("some data here");
		//ScopedDereferencer dr(data);
		//ScopedDereferencer tod(testObject);

		binding->Fire(event,data);
		KR_ASSERT_STR(testObject->Event().c_str(),"kr.api.log");
		KR_ASSERT_STR(testObject->Message().c_str(),"some data here");

		testObject->Reset();

		// TEST unregister and then refire -- we shouldn't receive it
		binding->Unregister(ref);

		binding->Fire(event,data);
		KR_ASSERT_STR(testObject->Event().c_str(),"");
		KR_ASSERT_STR(testObject->Message().c_str(),"");
	}
}
