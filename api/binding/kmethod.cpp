/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <sstream>
#include <cmath>

namespace kroll
{

	SharedString KMethod::DisplayString(int levels)
	{
		std::ostringstream oss;
		oss << "<KMethod at " << this << ">";
		return new std::string(oss.str());
	}

	SharedValue KMethod::Call()
	{
		return this->Call(ValueList());
	}

	SharedValue KMethod::Call(
		const char *one,
		SharedValue two,
		SharedValue three,
		SharedValue four)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		args.push_back(two);
		args.push_back(three);
		args.push_back(four);
		return this->Call(args);
	}

	SharedValue KMethod::Call(SharedValue one)
	{
		ValueList args;
		args.push_back(one);
		return this->Call(args);
	}

	SharedValue KMethod::Call(SharedValue one, SharedValue two)
	{
		ValueList args;
		args.push_back(one);
		args.push_back(two);
		return this->Call(args);
	}

	SharedValue KMethod::Call(
		SharedValue one,
		SharedValue two,
		SharedValue three)
	{
		ValueList args;
		args.push_back(one);
		args.push_back(two);
		args.push_back(three);
		return this->Call(args);
	}

	SharedValue KMethod::Call(const char *one)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		return this->Call(args);
	}

	SharedValue KMethod::Call(const char *one, SharedValue two)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		args.push_back(two);
		return this->Call(args);
	}

	SharedValue KMethod::Call(const char *one, SharedValue two, SharedValue three)
	{
		ValueList args;
		args.push_back(Value::NewString(one));
		args.push_back(two);
		args.push_back(three);
		return this->Call(args);
	}

}

