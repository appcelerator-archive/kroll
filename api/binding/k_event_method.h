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
			referenceCount(1) {}

		// @see KMethod::Call
		virtual SharedValue Call(const ValueList& args) = 0;

		// @see KMethod::Set
		virtual void Set(const char *name, SharedValue value)
		{
			KEventObject::Set(name, value);
		}

		// @see KMethod::Get
		virtual SharedValue Get(const char *name)
		{
			return KEventObject::Get(name);
		}

		// @see KMethod::GetPropertyNames
		virtual SharedStringList GetPropertyNames()
		{
			return KEventObject::GetPropertyNames();
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
		void SetMethod(const char *name, void (T::*method)(const ValueList&, SharedValue))
		{
			MethodCallback* callback = NewCallback<T, const ValueList&, SharedValue>(static_cast<T*>(this), method);

			SharedKMethod bound_method = new StaticBoundMethod(callback);
			SharedValue method_value = Value::NewMethod(bound_method);
			KEventObject::Set(name, method_value);
		}

		virtual void duplicate()
		{
			referenceCount++;
		}

		virtual void release()
		{
			referenceCount--;
			if (referenceCount.value() <= 0) {
				delete this;
			}
		}

		private:
		Poco::AtomicCounter referenceCount;

	};

}

#endif
