/**
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
	/*
		Class: StaticBoundObject
	*/
	class KROLL_API DelegateStaticBoundObject : public StaticBoundObject
	{
	public:
		/*
			Constructor: StaticBoundObject
		*/
		DelegateStaticBoundObject(SharedBoundObject delegate);
		virtual ~DelegateStaticBoundObject();

		/*
		  Function: Get

		  Return an object's property. The returned value is automatically
		  reference counted and must be released if the callee does not hold
		  a reference (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue Get(const char *name);

		/*
		  Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames();

		/*
		  Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, SharedValue value);

	private:
		SharedBoundObject delegate;
		Mutex mutex;
		DISALLOW_EVIL_CONSTRUCTORS(DelegateStaticBoundObject);
	};

}

#endif
