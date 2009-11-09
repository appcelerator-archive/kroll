/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_BOUND_METHOD_H_
#define _KR_PROFILED_BOUND_METHOD_H_

namespace kroll
{
	/**
	 * The ProfiledBoundMethod is a wrapped KMethod that does profiling
	 */
	class ProfiledBoundMethod : public ProfiledBoundObject, public KMethod
	{
	public:
		ProfiledBoundMethod(KMethodRef delegate, std::string& parentType);
		virtual ~ProfiledBoundMethod();

		// @see KMethod::Call
		virtual KValueRef Call(const ValueList& args);
		// @see KMethod::Set
		virtual void Set(const char *name, KValueRef value);
		// @see KMethod::Get
		virtual KValueRef Get(const char *name);
		// @see KMethod::GetPropertyNames
		virtual SharedStringList GetPropertyNames();
		// @see KObject::GetType
		virtual std::string& GetType();

		bool HasProperty(const char* name);

		/**
		 * @return the delegate of this profiled bound method
		 */
		KMethodRef GetDelegate() { return method; }
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
		KMethodRef method;
		std::string fullType;
		Poco::AtomicCounter count;

	};
}

#endif
