/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_METHOD_H_
#define _KR_ACCESSOR_BOUND_METHOD_H_

namespace kroll
{
	/**
	 * The AccessorBoundMethod allows you to expose a method with properties that automatically expose getters and setters
	 * @see AccessorBoundObject
	 */
	class KROLL_API AccessorBoundMethod : public StaticBoundMethod
	{
	public:

		AccessorBoundMethod(MethodCallback* callback, const char *type = "AccessorBoundMethod");

		/**
		 * @see KObject::Set
		 */
		virtual void Set(const char *name, SharedValue value);

		/**
		 * @see KObject::Get
		 */
		virtual SharedValue Get(const char *name);
		virtual bool HasProperty(const char *name);
	
		SharedValue RawGet(const char *name);
		void RawSet(const char *name, SharedValue value);

	private:
		SharedKMethod FindMethod(std::string& name);
		
		DISALLOW_EVIL_CONSTRUCTORS(AccessorBoundMethod);
	};
}
#endif
