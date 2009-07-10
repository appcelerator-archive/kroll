/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_KMETHOD_H_
#define _KJS_KMETHOD_H_

#include "javascript_module.h"

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	class KROLL_JAVASCRIPT_API KKJSMethod : public KMethod
	{
		public:
		KKJSMethod(JSContextRef, JSObjectRef, JSObjectRef);
		~KKJSMethod();

		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		SharedValue Call(const ValueList& args);
		SharedStringList GetPropertyNames();
		bool SameContextGroup(JSContextRef c);
		JSObjectRef GetJSObject();

		/*
		 * Determine if the given KJS object equals this one
		 * by comparing these objects via strict equality (===)
		 *  @param other the object to test
		 *  @returns true if objects have strit equality, false otherwise
		 */
		virtual bool Equals(SharedKObject);

		protected:
		JSGlobalContextRef context;
		JSObjectRef object;
		JSObjectRef this_obj;
		AutoPtr<KKJSObject> kjs_bound_object;

		private:
		DISALLOW_EVIL_CONSTRUCTORS(KKJSMethod);
	};
}

#endif
