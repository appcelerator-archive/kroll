/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_bound_method.h"

namespace kroll
{
	RubyBoundMethod::RubyBoundMethod(const char *n) : name(NULL)
	{
		if (n)
		{
			name = strdup(n);
		}
	}

	RubyBoundMethod::~RubyBoundMethod()
	{
		if (this->name) free(this->name);
		this->name = NULL;
	}

	Value* RubyBoundMethod::Call(const ValueList& args)
	{
		return Value::Undefined();
	}

	void RubyBoundMethod::Set(const char *name, Value* value)
	{
	}

	Value* RubyBoundMethod::Get(const char *name)
	{
		// // get should returned undefined if we don't have a property
		// // named "name" to mimic what happens in Javascript
		// if (0 == (PyObject_HasAttrString(this->object,(char*)name)))
		// {
		// 	return Value::Undefined();
		// }

		return Value::Undefined();
	}

	void RubyBoundMethod::GetPropertyNames(std::vector<const char *> *property_names)
	{
	}

}
