/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_BOUND_METHOD_H_
#define _KR_PROFILED_BOUND_METHOD_H_

#include "profiled_bound_object.h"

namespace kroll
{
	/**
	 * The ProfiledBoundMethod is a wrapped KMethod that does profiling
	 */
	class ProfiledBoundMethod : public ProfiledBoundObject, public KMethod
	{
	public:
		ProfiledBoundMethod(std::string name, SharedKMethod delegate, Poco::FileOutputStream *stream);
		virtual ~ProfiledBoundMethod();
	private:
		SharedKMethod method;
	public:
		/**
		 * Call this method with the given arguments.
		 * Errors will result in a thrown ValueException
		 * @return the return value of this method
		 */
		virtual SharedValue Call(const ValueList& args);

		/**
		 * Set a property on this object to the given value
		 * Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, SharedValue value);

		/**
		 * @return the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name);

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames();
	};
}

#endif