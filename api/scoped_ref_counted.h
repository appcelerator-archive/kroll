/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_SCOPED_REF_COUNTED_H_
#define _KR_SCOPED_REF_COUNTED_H_

#include "base.h"
#include "ref_counted.h"

namespace kroll
{
	/**
	 * utility class for incrementing and decrementing the reference
	 * count of the contained RefCounted object. this class is usually
	 * constructed as a stack variable and ensures that the held reference
	 * is available for the entire scope of the function call stack.  once
	 * the stack is deleted, the reference is released.
	 */
	class KROLL_API ScopedRefCounted
	{
	public:
		ScopedRefCounted(RefCounted *p);
		~ScopedRefCounted();
	protected:
		RefCounted *p;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(ScopedRefCounted);
	};
}
#endif
