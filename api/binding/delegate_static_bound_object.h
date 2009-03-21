/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_DELEGATE_STATIC_BOUND_OBJECT_H_
#define _KR_DELEGATE_STATIC_BOUND_OBJECT_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{

	class KROLL_API DelegateStaticBoundObject : public BoundObject
	{
	public:

		DelegateStaticBoundObject(SharedKObject delegate);
		DelegateStaticBoundObject(SharedKObject base, SharedKObject delegate);
		virtual ~DelegateStaticBoundObject();


		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		virtual void Set(const char *name, SharedValue value);

	private:

		/**
		 * The base part of this delegate object. This object
		 * is the first object searched for properties and also
		 * the targets of property assignments.
		 */
		SharedKObject base;

		/**
		 * The delegate part of this delegate object. This object
		 * is used to find properties if they are not found in the base.
		 */
		SharedKObject delegate;

		DISALLOW_EVIL_CONSTRUCTORS(DelegateStaticBoundObject);

	protected:
		Mutex mutex;

	};

}

#endif
