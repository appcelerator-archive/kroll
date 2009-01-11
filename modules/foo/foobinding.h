/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef FOO_BINDING_H_
#define FOO_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

namespace kroll
{
	class FooBinding : public StaticBoundObject
	{
	public:
		FooBinding();

		void Bar(const ValueList& args, Value *result);
	};
}

#endif
