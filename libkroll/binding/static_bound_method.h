/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_STATIC_BOUND_METHOD_H_
#define _KR_STATIC_BOUND_METHOD_H_

namespace kroll
{

	class KROLL_API StaticBoundMethod : public KMethod
	{
	public:

		StaticBoundMethod(MethodCallback* callback, const char *type = "StaticBoundMethod");
		virtual ~StaticBoundMethod();

		/**
		 * @see KMethod::Call
		 */
		virtual KValueRef Call(const ValueList& args);

		/**
		 * @see KObject::Set
		 */
		virtual void Set(const char *name, KValueRef value);

		/**
		 * @see KObject::Get
		 */
		virtual KValueRef Get(const char *name);

		/**
		 * @see KObject::GetPropertyNames
		 */
		virtual SharedStringList GetPropertyNames();
		
		/**
		 * Set a property on this object to the given method. When an error
		 * occurs will throw an exception of type ValueException.
		 */
		template <typename T>
		void SetMethod(const char *name, void (T::*method)(const ValueList&, KValueRef))
		{
			MethodCallback* callback = NewCallback<T, const ValueList&, KValueRef>(static_cast<T*>(this), method);

			KMethodRef bound_method = new StaticBoundMethod(callback);
			KValueRef method_value = Value::NewMethod(bound_method);
			this->Set(name, method_value);
		}

		template <typename T>
		static AutoPtr<StaticBoundMethod> FromMethod(T* owner, void (T::*method)(const ValueList&, KValueRef))
		{
			MethodCallback* callback = NewCallback<T, const ValueList&, KValueRef>(static_cast<T*>(owner), method);
			return new StaticBoundMethod(callback);
		}

	protected:
		SharedPtr<MethodCallback> callback;
		AutoPtr<StaticBoundObject> object;
		std::map<std::string, KValueRef > properties;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(StaticBoundMethod);
	};
}

#endif
