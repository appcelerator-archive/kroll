/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __PYTHON_TESTSUITE_H__
#define __PYTHON_TESTSUITE_H__

#include "python_module.h"

namespace kroll
{
	class KROLL_PYTHON_API PythonUnitTestSuite
	{
	public:
		void Run(Host*);
	};
}

#endif



