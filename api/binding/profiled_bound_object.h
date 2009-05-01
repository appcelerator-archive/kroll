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
	class ProfiledBoundObject : public KObject
	{
	public:
		ProfiledBoundObject(std::string name, SharedKObject delegate, Poco::FileOutputStream *stream);
		virtual ~ProfiledBoundObject();
	protected:
		SharedKObject delegate;
		std::string name;
		Poco::FileOutputStream *stream;
	public:
		/**
		 * Set a property on this object to the given value. Value should be
		 * heap-allocated as implementors are allowed to keep a reference.
		 * When an error occurs will throw an exception of type ValueException.
		 * @param name The property name
		 * @param value The new property value
		 */
		virtual void Set(const char *name, SharedValue value);

		/**
		 * @param name The property name
		 * @return the value of the property with the given name or Value::Undefined
		 * if the property is not found.
		 * Errors will result in a thrown ValueException
		 */
		virtual SharedValue Get(const char *name);

		/**
		 * @return a list of this object's property names.
		 */
		virtual SharedStringList GetPropertyNames();

	protected:
		
		/**
		 * get the full path to this object
		 */
		std::string GetFullPath()
		{
			return this->name;
		}
		static std::string MakeFullPath(ProfiledBoundObject* ref,std::string name);
		SharedValue ProfiledCall(ProfiledBoundObject* ref, SharedKMethod method, const ValueList& args);
		void Log(std::string str);
		static SharedValue Wrap(ProfiledBoundObject* source, const char *name, SharedValue value, ProfiledBoundObject **out);
	};
}

#endif
