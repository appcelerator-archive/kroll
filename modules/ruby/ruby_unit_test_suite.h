/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __RUBY_TESTSUITE_H__
#define __RUBY_TESTSUITE_H__

#include "ruby_module.h"

namespace kroll
{
	class KROLL_RUBY_API RubyUnitTestSuite
	{
	public:
		void Run(Host*);
	};
}

#endif



