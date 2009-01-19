/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_OBJECT_H_
#define _KR_STATIC_BOUND_OBJECT_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	/*
		Class: StaticBoundObject
	*/
	class KROLL_API StaticBoundObject : public BoundObject
	{
	public:
		/*
			Constructor: StaticBoundObject
		*/
		StaticBoundObject();
		virtual ~StaticBoundObject();

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


		/*
		  Function: Unset

		  Unset the named property
		 */
		virtual void UnSet(const char *name);


		/*
		  Function: SetMethod

		  Set a property on this object to the given method. When an error
		  occurs will throw an exception of type Value*.
		*/
		template <typename T>
		void SetMethod(const char *name, void (T::*method)(const ValueList&, SharedValue))
		{
			MethodCallback* callback = NewCallback<T, const ValueList&, SharedValue>(static_cast<T*>(this), method);

			SharedBoundMethod bound_method = new StaticBoundMethod(callback);
			SharedValue method_value = new Value(bound_method);

			this->Set(name, method_value);
		}

		/*
		  Function: SetObject

		  Set a property on this object to the given object. Value should be
		  heap-allocated as implementors are allowed to keep a reference.
		  When an error occurs will throw an exception of type Value*.
		*/
		void SetObject(const char *name, SharedBoundObject object);

	protected:
		std::map<std::string, SharedValue> properties;

	private:
		Mutex mutex; // allow multiple levels of mutexes
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundObject);
	};

}

#endif
