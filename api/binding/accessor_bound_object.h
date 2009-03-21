/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ACCESSOR_BOUND_OBJECT_H_
#define _KR_ACCESSOR_BOUND_OBJECT_H_

namespace kroll
{
	/**
	 * AccessorBoundObjects automatically expose any property as a getter or setter
	 * For example, if you expose a property "length" via:
	 * <code>
	 * this->Set("length", lengthValue);
	 * </code>
	 *
	 * A caller could then call "getLength" or "setLength", as well as accessing the property directly.
	 */
	class KROLL_API AccessorBoundObject : public StaticBoundObject
	{
	public:
		AccessorBoundObject();

		/*
			Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference,
		  if they increase the reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, SharedValue value);

		/*
			Function: Get

		  return a named property. the returned value is automatically
		  reference counted and you must release the reference when finished
		  with the return value (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue Get(const char *name);

		SharedValue RawGet(const char *name);

		void RawSet(const char *name, SharedValue value);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(AccessorBoundObject);
		static std::string Capitalize(const char *word);
	};
}

#endif
