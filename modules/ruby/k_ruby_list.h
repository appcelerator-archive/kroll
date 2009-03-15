/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_RUBY_LIST_H_
#define _K_RUBY_LIST_H_

#include "ruby_module.h"

namespace kroll
{
	class KRubyList : public KList
	{
	public:
		KRubyList(VALUE);
		virtual ~KRubyList();

		void Append(SharedValue value);
		unsigned int Size();
		SharedValue At(unsigned int index);
		void SetAt(unsigned int index, SharedValue value);
		bool Remove(unsigned int index);
		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int);

		VALUE ToRuby();

	protected:
		VALUE list;
		SharedPtr<KRubyObject> object;
		DISALLOW_EVIL_CONSTRUCTORS(KRubyList);
	};
}
#endif

