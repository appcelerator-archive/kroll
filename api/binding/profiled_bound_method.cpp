/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"
#include <cstdio>
#include <cstring>

namespace kroll
{
	ProfiledBoundMethod::ProfiledBoundMethod(std::string name, SharedKMethod delegate, Poco::FileOutputStream *stream) : ProfiledBoundObject(name,delegate,stream), method(delegate)
	{
	}
	ProfiledBoundMethod::~ProfiledBoundMethod()
	{
	}
	/**
	 * Call this method with the given arguments.
	 * Errors will result in a thrown ValueException
	 * @return the return value of this method
	 */
	SharedValue ProfiledBoundMethod::Call(const ValueList& args)
	{
		return this->ProfiledCall(this,method,args);
	}

	/**
	 * Set a property on this object to the given value
	 * Errors will result in a thrown ValueException
	 */
	void ProfiledBoundMethod::Set(const char *name, SharedValue value)
	{
		method->Set(name,value);
	}

	/**
	 * @return the property with the given name or Value::Undefined
	 * if the property is not found.
	 * Errors will result in a thrown ValueException
	 */
	SharedValue ProfiledBoundMethod::Get(const char *name)
	{
		return method->Get(name);
	}

	/**
	 * @return a list of this object's property names.
	 */
	SharedStringList ProfiledBoundMethod::GetPropertyNames()
	{
		return method->GetPropertyNames();
	}
}
