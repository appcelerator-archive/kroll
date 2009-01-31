/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BOUND_METHOD_H_
#define _KR_BOUND_METHOD_H_
#include <cstdarg>

namespace kroll
{
	typedef Callback2<const ValueList&, SharedValue>::Type MethodCallback;

	/*
		Class: BoundMethod
	*/
	class KROLL_API BoundMethod : public BoundObject
	{

	public:
		/*
			Constructor: BoundMethod
		*/
		BoundMethod() {}
		virtual ~BoundMethod(){}

		/*
			Function: Call

		  invoke the bound method. the returned value is automatically
		  reference counted and you must release the reference when finished
		  with the return value (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue Call(const ValueList& args) = 0;

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

		  return a named property. the returned value is automatically
		  reference counted and you must release the reference when finished
		  with the return value (even for Undefined and Null types).
		  When an error occurs will throw an exception of type Value*.
		 */
		virtual SharedValue Get(const char *name) = 0;

		/*
			Function: GetPropertyNames

		  Return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;


		/*
			Function: Call

		  call the method with a variable list of Value* arguments
		  When an error occurs will throw an exception of type Value*.
		 */
		/*SharedValue Call(SharedValue first, ...)
		{
			ValueList args;
			va_list vaargs;
			va_start(vaargs,first);
			args.push_back(first);
			while(1)
			{
		      SharedValue a = va_arg(vaargs,SharedValue);
		      if (a.isNull()) break;
		      args.push_back(a);
			}
			va_end(vaargs);
			return this->Call(args);
		}*/
		//NOTE: this ideally above would be an operator() overload
		//so you could just method() invoke this function but it doesn't
		//compile on GCC with "cannot be used as function"

	private:
		DISALLOW_EVIL_CONSTRUCTORS(BoundMethod);
	};
}

#endif

