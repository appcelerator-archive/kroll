/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_METHOD_H_
#define _KR_ACCESSOR_BOUND_METHOD_H_

namespace kroll
{
	/**
	 * The KAccessorMethod allows you to expose getters and setters as property access.
	 * @see KAccessorObject
	 */
	class KROLL_API KAccessorMethod : public StaticBoundMethod, public KAccessor
	{
	public:
		KAccessorMethod(MethodCallback* callback, const char* type = "KAccessorMethod");
		virtual void Set(const char* name, KValueRef value);
		virtual KValueRef Get(const char* name);
		virtual bool HasProperty(const char* name);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KAccessorMethod);
	};
}
#endif
