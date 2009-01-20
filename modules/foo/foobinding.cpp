/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "foobinding.h"

namespace kroll
{
	FooBinding::FooBinding()
	{
		printf("foobinding: %i\n", (int) this);

		SharedValue number = Value::NewInt(42);
		this->Set("number", number);
		this->SetMethod("bar", &FooBinding::Bar);

		SharedPtr<BoundList> arr = new StaticBoundList();
		arr->Append(Value::NewString("one"));
		arr->Append(Value::NewString("42"));
		arr->Append(Value::NewString("false"));
		this->Set("list", Value::NewList(arr));

		//KR_DECREF(number);
		//KR_DECREF(arr);
	}

	void FooBinding::Bar(const ValueList& args, SharedValue result)
	{
		if (args.size() > 0 && args[0]->IsString()) {
			std::cout << "you passed me: " << args[0]->ToString() << std::endl;
		}
	}
}
