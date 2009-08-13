/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PHP_LIST_H_
#define _K_PHP_LIST_H_

#include "php_module.h"

namespace kroll
{
	class KPHPList : public KList
	{
	public:
		KPHPList(zval *list);
		virtual ~KPHPList();

		SharedValue Get(const char *name);
		void Set(const char *name, SharedValue value);
		virtual bool Equals(SharedKObject);
		SharedStringList GetPropertyNames();

		unsigned int Size();
		void Append(SharedValue value);
		virtual void SetAt(unsigned int index, SharedValue value);
		bool Remove(unsigned int index);
		SharedValue At(unsigned int index);

		zval* ToPHP();

	protected:
		zval *list;
		DISALLOW_EVIL_CONSTRUCTORS(KPHPList);
	};
}
#endif
