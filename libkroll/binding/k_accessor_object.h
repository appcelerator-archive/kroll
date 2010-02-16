/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_OBJECT_H_
#define _KR_ACCESSOR_BOUND_OBJECT_H_

namespace kroll
{
	class KROLL_API KAccessorObject : public StaticBoundObject, public KAccessor
	{
	public:
		KAccessorObject(const char* name = "KAccessorObject");
		virtual void Set(const char* name, KValueRef value);
		virtual KValueRef Get(const char* name);
		virtual bool HasProperty(const char* name);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KAccessorObject);
	};
}

#endif
