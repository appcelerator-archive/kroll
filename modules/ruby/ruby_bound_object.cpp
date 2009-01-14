/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_bound_object.h"

namespace kroll
{
	RubyBoundObject::RubyBoundObject()
	{
	}

	RubyBoundObject::~RubyBoundObject()
	{
	}

	void RubyBoundObject::Set(const char *name, Value* value)
	{
	}

	Value* RubyBoundObject::Get(const char *name)
	{
		// // get should returned undefined if we don't have a property
		// // named "name" to mimic what happens in Javascript
		// if (0 == (PyObject_HasAttrString(this->object,(char*)name)))
		// {
		// 	return Value::Undefined();
		// }

		return Value::Undefined();
	}

	void RubyBoundObject::GetPropertyNames(std::vector<const char *> *property_names)
	{
	}
}

