/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_BOUND_METHOD_H_
#define _KR_PROFILED_BOUND_METHOD_H_

namespace kroll
{
	/**
	 * The ProfiledBoundMethod is a wrapped KMethod that does profiling
	 */
	class ProfiledBoundMethod : public ProfiledBoundObject, public KMethod
	{
	public:
		ProfiledBoundMethod(SharedKMethod delegate, std::string& parentType);
		virtual ~ProfiledBoundMethod();

		// @see KMethod::Call
		virtual SharedValue Call(const ValueList& args);
		// @see KMethod::Set
		virtual void Set(const char *name, SharedValue value);
		// @see KMethod::Get
		virtual SharedValue Get(const char *name);
		// @see KMethod::GetPropertyNames
		virtual SharedStringList GetPropertyNames();
		// @see KObject::GetType
		virtual std::string& GetType();

		/**
		 * @return the delegate of this profiled bound method
		 */
		SharedKMethod GetDelegate() { return method; }

	private:
		SharedKMethod method;
		std::string fullType;

	};
}

#endif
