/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_METHOD_H_
#define _K_METHOD_H_
#include <cstdarg>

namespace kroll
{
	typedef Callback2<const ValueList&, SharedValue>::Type MethodCallback;

	/*
		Class: KMethod
	*/
	class KROLL_API KMethod : public KObject
	{

	public:
		/*
			Constructor: KMethod
		*/
		KMethod() {}
		virtual ~KMethod() {}

		/*
		 * Function: Call
		 *  Call this method with the given arguments.
		 *  Errors will result in a thrown ValueException
		 */
		virtual SharedValue Call(const ValueList& args) = 0;

		/*
		 * Function: Set
		 *   Set a property on this object to the given value
		 *   Errors will result in a thrown ValueException
		 */
		virtual void Set(const char *name, SharedValue value) = 0;

		/*
		 * Function: Get
		 *   Return the property with the given name or Value::Undefined
		 *   if the property is not found.
		 *   Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name) = 0;

		/*
		 * Function: GetPropertyNames
		 * Returns: a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/*
		 * Function: DisplayString
		 * Returns: a string representation of this object
		 */
		SharedString DisplayString(int levels);


		/* Convenience methods below */
		SharedValue Call(SharedValue one);
		SharedValue Call(SharedValue one, SharedValue two);
		SharedValue Call(SharedValue one, SharedValue two, SharedValue three);
		SharedValue Call();
		SharedValue Call(const char *one);
		SharedValue Call(const char *one, SharedValue two);
		SharedValue Call(const char *one, SharedValue two, SharedValue three);
		SharedValue Call(
			const char *one,
			SharedValue two,
			SharedValue three,
			SharedValue four);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KMethod);
	};
}

#endif

