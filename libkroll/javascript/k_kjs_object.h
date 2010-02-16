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
	class KROLL_API KKJSObject : public KObject
	{
		public:
		KKJSObject(JSContextRef context, JSObjectRef js_object);
		~KKJSObject();

		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		virtual bool HasProperty(const char* name);
		virtual bool Equals(KObjectRef);

		bool SameContextGroup(JSContextRef c);
		JSObjectRef GetJSObject();

		protected:
		JSGlobalContextRef context;
		JSObjectRef jsobject;

		private:
		DISALLOW_EVIL_CONSTRUCTORS(KKJSObject);
	};
}

#endif
