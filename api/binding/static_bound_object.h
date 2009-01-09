/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_OBJECT_H_
#define _KR_STATIC_BOUND_OBJECT_H_

#include "binding.h"

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	class KROLL_API StaticBoundObject : public BoundObject
	{
	public:
		StaticBoundObject();

	protected:
		virtual ~StaticBoundObject();

	public:
		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference.
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Get(const char *name, BoundObject *context_local);

		/**
		 * Return an object's property. The returned value is automatically
		 * reference counted and must be released if the callee does not hold
		 * a reference (even for Undefined and Null types).
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual Value* Get(const char *name);

		/**
		 * Return a list of this object's property names.
		 */
		virtual std::vector<std::string> GetPropertyNames();

		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference.
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, Value* value, BoundObject *context_local);

		/**
		 * set the named property to a value with a null BoundObject* context
		 * When an error occurs will throw an exception of type Value*.
		 */
		virtual void Set(const char *name, Value* value);


		/**
		 * unset the named property
		 */
		virtual void UnSet(const char *name);


		/* convenience methods for the conviencence methods */
		template <typename T>
		void SetMethod(const char *name, void (T::*method)(const ValueList&, Value*, BoundObject *context_local))
		{
			MethodCallback* callback = NewCallback<T, const ValueList&, Value*, BoundObject*>(static_cast<T*>(this), method);

			StaticBoundMethod* bound_method = new StaticBoundMethod(callback);
			Value* method_value = new Value(bound_method);

			this->Set(name, method_value);

			KR_DECREF(bound_method);
			KR_DECREF(method_value);
		}

		void SetObject(const char *name, BoundObject* object);

	protected:
		Mutex mutex;
		std::map<std::string, Value*> properties;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundObject);
	};
}

#endif
