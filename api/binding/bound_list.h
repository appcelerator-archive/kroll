
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_LIST_H_
#define _KR_BOUND_LIST_H_

#include <string>
#include <cctype>

namespace kroll
{
	/*
		Class: BoundList
	*/
	class KROLL_API BoundList : public BoundObject
	{
	protected:
		virtual ~BoundList(){}
	public:
		BoundList() {}

		/*
			Function: Append

		  Append a value to this list. Value should be heap-allocated as
		  implementors are allowed to keep a reference, if they increase the
		  reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Append(Value* value) = 0;

		/*
			Function: Size

		  Get the length of this list.
		 */
		virtual int Size() = 0;

		/*
			Function: At

		  When an error occurs will throw an exception of type Value*.
		  Return the value at the given index. The value is automatically
		  reference counted and must be released.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* At(int index) = 0;

		/*
			Function: Set

		  Set a property on this object to the given value. Value should be
		  heap-allocated as implementors are allowed to keep a reference,
		  if they increase the reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, Value* value) = 0;

		/*
			Function: Get

		  return a named property. the returned value is automatically
		  reference counted and you must release the reference when finished
		  with the return value (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Get(const char *name) = 0;

		/*
			Function: GetPropertyNames

			Returns:

		  a list of this object's property names.
		 */
		virtual void GetPropertyNames(std::vector<const char *> *property_names) = 0;

	protected:
		bool IsNumber (const char *name)
		{
			std::string s(name);
			for (int c=0;c<(int)s.length();c++)
			{
				if (!std::isdigit(s[c]))
				{
					return false;
				}
			}
			return true;
		}

	private:
		DISALLOW_EVIL_CONSTRUCTORS(BoundList);
	};
}

#endif
