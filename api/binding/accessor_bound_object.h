/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_OBJECT_H_
#define _KR_ACCESSOR_BOUND_OBJECT_H_

namespace kroll
{
	class KROLL_API AccessorBoundObject : public StaticBoundObject
	{
	public:
		AccessorBoundObject(const char* name = "");

		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual bool HasProperty(const char* name);

		SharedKMethod FindMethod(std::string&);
		SharedValue RawGet(const char *name);
		void RawSet(const char *name, SharedValue value);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(AccessorBoundObject);
	};
}

#endif
