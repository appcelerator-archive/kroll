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

		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		KValueRef Call(JSObjectRef thisObject, const ValueList& args);
		virtual KValueRef Call(const ValueList& args);
		virtual KValueRef Call(KObjectRef thisObject, const ValueList& args);
		virtual SharedStringList GetPropertyNames();
		virtual bool HasProperty(const char* name);
		virtual bool Equals(KObjectRef);

		virtual bool SameContextGroup(JSContextRef c);
		JSObjectRef GetJSObject();

		protected:
		JSGlobalContextRef context;
		JSObjectRef jsobject;
		JSObjectRef thisObject;
		AutoPtr<KKJSObject> kobject;

		private:
		DISALLOW_EVIL_CONSTRUCTORS(KKJSMethod);
	};
}

#endif
