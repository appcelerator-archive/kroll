/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_VOID_PTR_OBJECT_H_
#define _KR_VOID_PTR_OBJECT_H_

#include "../kroll.h"

namespace kroll
{
	/**
	 * An object that represents an arbitrary amount of binary data§
	 */
	class KROLL_API VoidPtr : public StaticBoundObject
	{
	public:
		VoidPtr(void* pointer) :
			StaticBoundObject("VoidPtr"),
			pointer(pointer) {}
		void* GetPtr() { return pointer; }

	private:
		void* pointer;
	};
}

#endif
