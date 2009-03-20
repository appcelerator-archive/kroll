/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_METHOD_H_
#define _KR_STATIC_BOUND_METHOD_H_

namespace kroll
{
	/*
		Class: StaticBoundMethod
	*/
	class KROLL_API StaticBoundMethod : public KMethod
	{
	public:
		/*
			Constructor: StaticBoundMethod
		*/
		StaticBoundMethod(MethodCallback*);
		virtual ~StaticBoundMethod();

		/*
			Function: Call

		  invoke a bound method. the returned value is automatically
		  reference counted and you must release the reference when finished
		  with the return value (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue Call(const ValueList& args);

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

	protected:
		SharedPtr<MethodCallback> callback;
		SharedPtr<StaticBoundObject> object;
		std::map<std::string, SharedValue > properties;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundMethod);
	};
}

#endif
