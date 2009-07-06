/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_PROFILED_BOUND_OBJECT_H_
#define _KR_PROFILED_BOUND_OBJECT_H_
#include <Poco/FileStream.h>

namespace kroll
{
	/**
	 * The ProfiledBoundObject is a wrapped KObject that does profiling on a 
	 * wrapped KObject
	 */
	class KROLL_API ProfiledBoundObject : public KObject
	{
		public:
		ProfiledBoundObject(SharedKObject delegate);
		virtual ~ProfiledBoundObject();
		static void SetStream(Poco::FileOutputStream*);

		public:
		// @see KObject::Set
		virtual void Set(const char *name, SharedValue value);
		// @see KObject::Get
		virtual SharedValue Get(const char *name);
		// @see KObject::GetPropertyNames
		virtual SharedStringList GetPropertyNames();
		// @see KObject::DisplayString
		virtual SharedString DisplayString(int levels=3);
		// @see KObject::Equals
		virtual bool Equals(SharedKObject other);

		/**
		 * @return the delegate of this profiled bound object
		 */
		SharedKObject GetDelegate() { return delegate; }
		
	protected:
		SharedKObject delegate;
		SharedValue Wrap(SharedValue value, std::string type);
		std::string GetSubType(std::string name);
		void Log(std::string eventType, std::string& name, Poco::Timestamp::TimeDiff);
		static bool AlreadyWrapped(SharedValue);
		static Poco::FileOutputStream *stream;
	};
}

#endif
