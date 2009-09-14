/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
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
	class KROLL_JAVASCRIPT_API KKJSList : public KList
	{

		public:
		KKJSList(JSContextRef context, JSObjectRef jsObject);
		~KKJSList();

		virtual void Set(const char *name, SharedValue value);
		virtual void SetAt(unsigned int index, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		virtual bool HasProperty(const char* name);
		virtual bool Equals(SharedKObject);
		virtual void Append(SharedValue value);
		virtual unsigned int Size();
		virtual SharedValue At(unsigned int index);
		virtual bool Remove(unsigned int index);

		bool SameContextGroup(JSContextRef c);
		JSObjectRef GetJSObject();

		protected:
		JSGlobalContextRef context;
		JSObjectRef jsobject;
		AutoPtr<KKJSObject> kobject;

		private:
		DISALLOW_EVIL_CONSTRUCTORS(KKJSList);
	};
}

#endif
