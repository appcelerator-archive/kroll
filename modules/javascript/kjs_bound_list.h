/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_BOUND_LIST_H_
#define _KJS_BOUND_LIST_H_

#include "javascript_module.h"

#include <vector>
#include <string>
#include <map>
#include <cmath>

namespace kroll
{
	class KJSBoundList : public BoundList
	{

	public:

		KJSBoundList(JSContextRef context,
		             JSObjectRef js_object);
	protected:
		~KJSBoundList();
	public:

		void Set(const char *name, Value* value);
		Value* Get(const char *name);
		void GetPropertyNames(std::vector<const char *> *property_names);
		bool SameContextGroup(JSContextRef c);

		void Append(Value* value);
		int Size();
		Value* At(int index);

		JSObjectRef GetJSObject();

	protected:
		JSContextRef context;
		JSObjectRef object;
		KJSBoundObject* kjs_bound_object;

		static char* IntToChars(int value);
	private:
                DISALLOW_EVIL_CONSTRUCTORS(KJSBoundList);

	};
}

#endif
