/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _RUBY_BOUND_OBJECT_H_
#define _RUBY_BOUND_OBJECT_H_

#include "ruby_module.h"

namespace kroll
{
	class RubyBoundObject : public BoundObject
	{
	public:
		RubyBoundObject();
	protected:
		virtual ~RubyBoundObject();
	public:
		virtual void Set(const char *name, Value* value);
		virtual Value* Get(const char *name);
		virtual void GetPropertyNames(std::vector<const char *> *property_names);

	private:
        DISALLOW_EVIL_CONSTRUCTORS(RubyBoundObject);
	};
}
#endif

