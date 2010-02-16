/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_METHOD_H_
#define _K_METHOD_H_
#include <cstdarg>

namespace kroll
{
	typedef Callback2<const ValueList&, KValueRef>::Type MethodCallback;

	/**
	 * An abstract representation of a method
	 */
	class KROLL_API KMethod : public KObject
	{

	public:

		KMethod(const char *type = "KMethod") : KObject(type) {}
		virtual ~KMethod() {}

		/**
		 * Call this method with the given arguments.
		 * Errors will result in a thrown ValueException
		 * @return the return value of this method
		 */
		virtual KValueRef Call(const ValueList& args) = 0;

		/**
		 * Call this method with the given 'this' object and arguments.
		 * Errors will result in a thrown ValueException
		 * @return the return value of this method
		 */
		virtual KValueRef Call(KObjectRef thisObject, const ValueList& args);

		/**
		 * Set a property on this object to the given value
		 * Errors will result in a thrown ValueException
		 */
		virtual void Set(const char* name, KValueRef value) = 0;

		/**
		 * @return the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual KValueRef Get(const char* name) = 0;

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames() = 0;

		/**
		 * @return a string representation of this object
		 */
		SharedString DisplayString(int levels);

		/**
		 * Return the unwrapped version of this object
		 */
		static KMethodRef Unwrap(KMethodRef);

		/* Convenience methods below */
		KValueRef Call(KValueRef one);
		KValueRef Call(KValueRef one, KValueRef two);
		KValueRef Call(KValueRef one, KValueRef two, KValueRef three);
		KValueRef Call();
		KValueRef Call(const char* one);
		KValueRef Call(const char* one, KValueRef two);
		KValueRef Call(const char* one, KValueRef two, KValueRef three);
		KValueRef Call(const char* one, KValueRef two, KValueRef three,
			KValueRef four);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KMethod);
	};
}

#endif

