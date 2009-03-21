/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_METHOD_H_
#define _KR_STATIC_BOUND_METHOD_H_

namespace kroll
{

	class KROLL_API StaticBoundMethod : public KMethod
	{
	public:

		StaticBoundMethod(MethodCallback*);
		virtual ~StaticBoundMethod();

		/**
		 * @see KMethod::Call
		 */
		virtual SharedValue Call(const ValueList& args);

		/**
		 * @see KObject::Set
		 */
		virtual void Set(const char *name, SharedValue value);

		/**
		 * @see KObject::Get
		 */
		virtual SharedValue Get(const char *name);

		/**
		 * @see KObject::GetPropertyNames
		 */
		virtual SharedStringList GetPropertyNames();

	protected:
		SharedPtr<MethodCallback> callback;
		SharedPtr<StaticBoundObject> object;
		std::map<std::string, SharedValue > properties;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundMethod);
	};
}

#endif
