/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_BOUND_OBJECT_H_
#define _KJS_BOUND_OBJECT_H_

#include "javascript_module.h"

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	class KJSBoundObject : public BoundObject
	{
	public:
		KJSBoundObject(JSContextRef context,
		               JSObjectRef js_object);
	protected:
		~KJSBoundObject();
	public:

		void Set(const char *name, Value* value);
		Value* Get(const char *name);
		void GetPropertyNames(std::vector<const char *> *property_names);
		bool SameContextGroup(JSContextRef c);

		JSObjectRef GetJSObject();

	protected:
		JSContextRef context;
		JSObjectRef object;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KJSBoundObject);

	};
}

#endif
