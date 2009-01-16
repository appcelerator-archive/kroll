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

		SharedPtr<Value> number = new Value(42);
		this->Set("number", number);
		this->SetMethod("bar", &FooBinding::Bar);

		SharedPtr<BoundList> arr = new StaticBoundList();
		arr->Append(new Value("one"));
		arr->Append(new Value("42"));
		arr->Append(new Value("false"));
		this->Set("list", new Value(arr));

		//KR_DECREF(number);
		//KR_DECREF(arr);
	}

	void FooBinding::Bar(const ValueList& args, SharedPtr<Value> result)
	{
		if (args.size() > 0 && args[0]->IsString()) {
			std::cout << "you passed me: " << args[0]->ToString() << std::endl;
		}
	}
}
