/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_EVENT_METHOD_H_
#define _KR_EVENT_METHOD_H_


namespace kroll
{
	class KROLL_API KEventMethod : public KEventObject, public KMethod
	{
		public:
		KEventMethod(const char* name = "") :
			KEventObject(name),
			count(1) {}

		// @see KMethod::Call
		virtual KValueRef Call(const ValueList& args) = 0;

		// @see KMethod::Set
		virtual void Set(const char *name, KValueRef value)
		{
			KEventObject::Set(name, value);
		}

		// @see KMethod::Get
		virtual KValueRef Get(const char *name)
		{
			return KEventObject::Get(name);
		}

		// @see KMethod::GetPropertyNames
		virtual SharedStringList GetPropertyNames()
		{
			return KEventObject::GetPropertyNames();
		}

		// @see KMethod::HasProperty
		virtual bool HasProperty(const char *name)
		{
			return KEventObject::HasProperty(name);
		}
		
		// @see KMethod::DisplayString
		SharedString DisplayString(int levels)
		{
			return KEventObject::DisplayString(levels);
		}

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
			KEventObject::Set(name, method_value);
		}

		virtual void duplicate()
		{
			++count;
		}

		virtual void release()
		{
			int value = --count;
			if (value <= 0) {
				delete this;
			}
		}

		virtual int referenceCount() const
		{
			return count.value();
		}

		private:
		Poco::AtomicCounter count;

	};

}

#endif
