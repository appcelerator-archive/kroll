/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_DELEGATE_STATIC_BOUND_OBJECT_H_
#define _KR_DELEGATE_STATIC_BOUND_OBJECT_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	/**
	 * The DelegateStaticBoundObject lets you wrap a globally accessible
	 * object in such a way as it appears to have special properties in
	 * in local contexts. When a Get(...) occurs, the object searches a local
	 * delegate and then a global one. When a Set(...) occurs, the object
	 * performs it on both the local and the global version. Thus this object
	 * is most useful if the local properties are assigned to the local object
	 * in an initial setup phase.
	 */
	class KROLL_API DelegateStaticBoundObject : public KObject
	{
		public:
		DelegateStaticBoundObject(SharedKObject global);
		DelegateStaticBoundObject(SharedKObject global, SharedKObject local);
		virtual ~DelegateStaticBoundObject();
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		virtual void Set(const char *name, SharedValue value);
		virtual bool HasProperty(const char* name);

		virtual inline SharedKObject GetGlobal() { return this->global; }
		virtual inline SharedKObject GetLocal() { return this->local; }

		private:
		/**
		 * The global part of this delegate object. This object
		 * is used to find properties if they are not found in
		 * the local object.
		 */
		SharedKObject global;

		/**
		 * The local part of this delegate object. This object
		 * is the first in line for property retrieval. 
		 */
		SharedKObject local;

		DISALLOW_EVIL_CONSTRUCTORS(DelegateStaticBoundObject);

	protected:
		Mutex mutex;
	};
}

#endif
