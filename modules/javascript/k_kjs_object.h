/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_KOBJECT_H_
#define _KJS_KOBJECT_H_

#include "javascript_module.h"

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	class KROLL_JAVASCRIPT_API KKJSObject : public KObject
	{
	public:
		KKJSObject(JSContextRef context, JSObjectRef js_object);
		~KKJSObject();

		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
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

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KKJSObject);

	};
}

#endif
