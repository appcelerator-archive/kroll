/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <string.h>
#include <iostream>
#include <vector>
#include "api_module.h"
#include "api_unit_test_suite.h"

namespace kroll
{
	KROLL_MODULE(APIModule)

	void APIModule::Initialize()
	{
		binding = new APIBinding(host->GetGlobalObject());
		host->GetGlobalObject()->SetObject("API", binding);
	}

	void APIModule::Stop()
	{
		host->GetGlobalObject()->Set("API", Value::Undefined);
	}

	void APIModule::Test()
	{
		APIUnitTestSuite suite;
		suite.Run(host, binding);
	}
}
