/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "kroll.h"

namespace kroll
{
	ScopedDereferencer::ScopedDereferencer(RefCounted *p) : p(p)
	{
		//DON'T INCREMENT!
	}

	ScopedDereferencer::~ScopedDereferencer()
	{
		KR_DECREF(p);
	}
}

