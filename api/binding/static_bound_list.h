/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_LIST_H_
#define _KR_STATIC_BOUND_LIST_H_

namespace kroll
{
	/*
		Class: StaticBoundList
	*/
	class KROLL_API StaticBoundList : public BoundList
	{
	public:
		/*
			Constructor: StaticBoundList
		*/
		StaticBoundList();
		virtual ~StaticBoundList();
		static SharedBoundList FromStringVector(std::vector<std::string>&);

		/*
			Function: Append

		  Append a value to this list. Value should be heap-allocated as
		  implementors are allowed to keep a reference, if they increase the
		  reference count.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual void Append(SharedValue value);

		/*
			Function: Size

		  Get the length of this list.
		 */
		virtual int Size();

		/*
			Function: At

		  When an error occurs will throw an exception of type Value*.
		  Return the value at the given index. The value is automatically
		  reference counted and must be released.
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue At(unsigned int index);

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

		/*
		    Function: Remove
		    
		    remove the entry at index and returns true if found and removed
		 */
		virtual bool Remove(unsigned int index);

		/*
			Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames();

	protected:
		static char* IntToChars(unsigned int value);
		SharedPtr<StaticBoundObject> object;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundList);
	};
}

#endif
