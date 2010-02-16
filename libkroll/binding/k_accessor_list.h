/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_LIST_H_
#define _KR_ACCESSOR_BOUND_LIST_H_

namespace kroll
{
	/**
	 * The KAccessorList allows you to expose getters and setters as property access.
	 * @see KAccessorObject
	 */
	class KROLL_API KAccessorList : public StaticBoundList, public KAccessor
	{
	public:
		KAccessorList(const char* type = "KAccessorList");
		virtual void Set(const char* name, KValueRef value);
		virtual KValueRef Get(const char* name);
		virtual bool HasProperty(const char* name);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KAccessorList);
	};
}

#endif
