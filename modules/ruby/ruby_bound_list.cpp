/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_bound_list.h"

namespace kroll
{
	RubyBoundList::RubyBoundList()
	{
	}

	RubyBoundList::~RubyBoundList()
	{
	}

	/**
	 * Append a value to this list. Value should be heap-allocated as
	 * implementors are allowed to keep a reference, if they increase the
	 * reference count.
	 * When an error occurs will throw an exception of type Value*.
	 */
	void RubyBoundList::Append(Value* value)
	{
	}

	/**
	 * Get the length of this list.
	 */
	int RubyBoundList::Size()
	{
		return 0;
	}

	/**
	 * When an error occurs will throw an exception of type Value*.
	 * Return the value at the given index. The value is automatically
	 * reference counted and must be released.
	 * When an error occurs will throw an exception of type Value*.
	 */
	Value* RubyBoundList::At(int index)
	{
		return Value::Undefined();
	}

	/**
	 * Set a property on this object to the given value. Value should be
	 * heap-allocated as implementors are allowed to keep a reference,
	 * if they increase the reference count.
	 * When an error occurs will throw an exception of type Value*.
	 */
	void RubyBoundList::Set(const char *name, Value* value)
	{
		// check for integer value as name
		if (this->IsNumber(name))
		{
			int val = atoi(name);
			if (val >= this->Size())
			{
				// now we need to create entries
				// between current size and new size
				// and make the entries null
				for (int c = this->Size(); c <= val; c++)
				{
					this->Append(NULL);
				}
			}

			Value* current = this->At(val);
			current->Set(value);
		}
		else
		{
		}
	}

	/**
	 * return a named property. the returned value is automatically
	 * reference counted and you must release the reference when finished
	 * with the return value (even for Undefined and Null types).
	 * When an error occurs will throw an exception of type Value*.
	 */
	Value* RubyBoundList::Get(const char *name)
	{
		if (std::string(name) == std::string("length"))
		{
			return new Value(this->Size());
		}
		return Value::Undefined();
	}

	/**
	 * Return a list of this object's property names.
	 */
	void RubyBoundList::GetPropertyNames(std::vector<const char *> *property_names)
	{
		property_names->push_back("length");
	}
}
