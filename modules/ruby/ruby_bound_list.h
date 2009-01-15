/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _RUBY_BOUND_LIST_H_
#define _RUBY_BOUND_LIST_H_

#include "ruby_module.h"

namespace kroll
{
	class RubyBoundList : public BoundList
	{
	public:
		RubyBoundList();

		/**
		 * Append a value to this list. Value should be heap-allocated as
		 * implementors are allowed to keep a reference, if they increase the
		 * reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		void Append(Value* value);

		/**
		 * Get the length of this list.
		 */
		int Size();

		/**
		 * When an error occurs will throw an exception of type Value*.
		 * Return the value at the given index. The value is automatically
		 * reference counted and must be released.
		 * When an error occurs will throw an exception of type Value*.
		 */
		Value* At(int index);

		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference,
		 * if they increase the reference count.
		 * When an error occurs will throw an exception of type Value*.
		 */
		void Set(const char *name, Value* value);

		/**
		 * return a named property. the returned value is automatically
		 * reference counted and you must release the reference when finished
		 * with the return value (even for Undefined and Null types).
		 * When an error occurs will throw an exception of type Value*.
		 */
		Value* Get(const char *name);

		/**
		 * Return a list of this object's property names.
		 */
		void GetPropertyNames(std::vector<const char *> *property_names);

	protected:
		virtual ~RubyBoundList();
        DISALLOW_EVIL_CONSTRUCTORS(RubyBoundList);
	};
}
#endif

