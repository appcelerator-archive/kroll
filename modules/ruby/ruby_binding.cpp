/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "rubybinding.h"
#include "rubytypes.h"
#include <string>

namespace kroll
{
	RubyBinding::RubyBinding(VALUE ruby_object_) : ruby_object(ruby_object_)
	{
		BindDynamicMethod(&RubyBinding::RubyInvoke);
		BindDynamicGetter(&RubyBinding::RubyGet);
		BindDynamicSetter(&RubyBinding::RubySet);
	}

	void RubyBinding::RubyInvoke(const char *name, const ArgList& args, ReturnValue *returnValue)
	{
		std::cout << "RubyInvoke " << name << std::endl;

		VALUE response =
			rb_funcall2(ruby_object, rb_intern(name), args.size(),
				ArgListToCValueArray(args));

		returnValue->Set(RubyValueToArgValue(response));
	}

	void RubyBinding::RubyGet(const char *name, ReturnValue *returnValue)
	{
		std::string varname = "@";
		varname+= name;

		VALUE value = rb_iv_get(ruby_object, varname.c_str());

		returnValue->Set(RubyValueToArgValue(value));
	}

	void RubyBinding::RubySet(const char *name, const ArgValue& value)
	{
		std::string varname = "@";
		varname+= name;

		ArgValue v = value;
		rb_iv_set(ruby_object, varname.c_str(), ArgValueToRubyValue(v));
	}

	void RubyModuleInstance::Initialize()
	{

	}

	void RubyModuleInstance::Destroy()
	{

	}
}
