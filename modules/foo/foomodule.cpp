/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "foomodule.h"
#include <iostream>

namespace kroll
{
	KROLL_MODULE(FooModule)

	void FooModule::Initialize()
	{
		binding = new FooBinding();
		host->GetGlobalObject()->SetObject("foo", binding);
	}

	void FooModule::Stop()
	{
	}
}
