/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _RUBY_BOUND_METHOD_H_
#define _RUBY_BOUND_METHOD_H_

#include "ruby_module.h"

namespace kroll
{
	class RubyBoundMethod : public BoundMethod
	{
	public:
		RubyBoundMethod(const char *name);
	protected:
		virtual ~RubyBoundMethod();
	public:
		Value* Call(const ValueList& args);
		virtual void Set(const char *name, Value* value);
		virtual Value* Get(const char *name);
		virtual void GetPropertyNames(std::vector<const char *> *property_names);

	private:
		char* name;
        DISALLOW_EVIL_CONSTRUCTORS(RubyBoundMethod);
	};
}

#endif

