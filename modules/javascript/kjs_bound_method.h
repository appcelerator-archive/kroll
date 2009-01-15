/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_BOUND_METHOD_H_
#define _KJS_BOUND_METHOD_H_

#include "javascript_module.h"

#include <vector>
#include <string>
#include <map>

namespace kroll
{
	class KROLL_JAVASCRIPT_API KJSBoundMethod : public BoundMethod
	{

	public:

		KJSBoundMethod(JSContextRef context,
		               JSObjectRef js_object,
		               JSObjectRef this_obj);
	protected:
		~KJSBoundMethod();
	public:

		void Set(const char *name, Value* value);
		Value* Get(const char *name);
		Value* Call(const ValueList& args);
		void GetPropertyNames(std::vector<const char *> *property_names);
		bool SameContextGroup(JSContextRef c);
		JSObjectRef GetJSObject();

	protected:
		JSContextRef context;
		JSObjectRef object;
		JSObjectRef this_obj;
		KJSBoundObject* kjs_bound_object;


	private:
		DISALLOW_EVIL_CONSTRUCTORS(KJSBoundMethod);
	
	};
}

#endif
