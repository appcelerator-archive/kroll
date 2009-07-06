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
		ProfiledBoundObject(std::string name, SharedKObject delegate, Poco::FileOutputStream *stream);
		virtual ~ProfiledBoundObject();

		protected:
		SharedKObject delegate;
		std::string name;
		Poco::FileOutputStream *stream;

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
		
		/**
		 * get the full path to this object
		 */
		std::string GetFullPath()
		{
			return this->name;
		}
		static std::string MakeFullPath(ProfiledBoundObject* ref, std::string name, bool useParent);
		SharedValue ProfiledCall(ProfiledBoundObject* ref, SharedKMethod method, const ValueList& args);
		void Log(std::string str);
		static SharedValue Wrap(ProfiledBoundObject* source, std::string name, SharedValue value, ProfiledBoundObject **out, bool useParent=false);
	};
}

#endif
