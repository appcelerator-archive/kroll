/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __API_TESTSUITE_H__
#define __API_TESTSUITE_H__

#include "api_module.h"

namespace kroll
{
	class KROLL_API_API APIUnitTestSuite
	{
	public:
		void Run(Host*, SharedKObject);
	};
}

#endif



