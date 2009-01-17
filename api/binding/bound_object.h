/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_OBJECT_H_
#define _KR_BOUND_OBJECT_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	/*
		Class: BoundObject
	*/
	class KROLL_API BoundObject : public RefCounted
	{
	public:
		/*
			Constructor: BoundObject
		*/
		BoundObject() { }
		virtual ~BoundObject() { }

		static SharedBoundObject CreateEmptyBoundObject();
	public:
		/*
			Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference,
		  if they increase the reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, SharedValue value) = 0;

		/*
			Function: Get

		  Return an object's property. The returned value is automatically
		  reference counted and must be released if the callee does not hold
		  a reference (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue Get(const char *name) = 0;

		/*
			Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;


		/* Function: Set

			Helpful overload to Set which takes a SharedString
		 */
		void Set(SharedString, SharedValue value);

		/* Function: Get

			Helpful overload to Get which takes a SharedString
		 */
		SharedValue Get(SharedString);

		/*
			Function: SetNS

			TODO: Document me
		*/
		void SetNS(const char *name, SharedValue value);

		/*
			Function: GetNS

			TODO: Document me
		*/
		SharedValue GetNS(const char *name);

		/**
		 * Function: DisplayString
		 *
		 * Return a string representation of this object
		 */
		SharedString DisplayString(int levels=3);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(BoundObject);
	};

}

#endif
