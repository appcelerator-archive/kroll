/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_KLIST_H_
#define _KJS_KLIST_H_

#include "javascript_module.h"

#include <vector>
#include <string>
#include <map>
#include <cmath>

namespace kroll
{
	class KROLL_JAVASCRIPT_API KJSKList : public KList
	{

	public:

		KJSKList(JSContextRef context, JSObjectRef js_object);
		~KJSKList();

		void Set(const char *name, SharedValue value);
		void SetAt(unsigned int index, SharedValue value);
		SharedValue Get(const char *name);
		SharedStringList GetPropertyNames();
		bool SameContextGroup(JSContextRef c);

		void Append(SharedValue value);
		unsigned int Size();
		SharedValue At(unsigned int index);
		bool Remove(unsigned int index);

		JSObjectRef GetJSObject();

	protected:
		JSGlobalContextRef context;
		JSObjectRef object;
		SharedPtr<KJSKObject> kjs_bound_object;

	private:
		DISALLOW_EVIL_CONSTRUCTORS(KJSKList);

	};
}

#endif
