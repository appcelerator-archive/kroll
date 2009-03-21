/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_SCOPED_DEREFERENCER_H_
#define _KR_SCOPED_DEREFERENCER_H_

#include "base.h"
#include "ref_counted.h"

namespace kroll
{
	/**
	 * utility class for handling calling ReleaseReference() on the contained
	 * RefCounted object.  However, the constructor of this class *does not*
	 * call AddReference(). It is commonly used to release a reference once
	 * the object is destructed.
	 */
	class KROLL_API ScopedDereferencer
	{
	public:

		ScopedDereferencer(RefCounted *p);
		~ScopedDereferencer();
	protected:
		RefCounted *p;
	private:
		DISALLOW_EVIL_CONSTRUCTORS(ScopedDereferencer);
	};
}
#endif
