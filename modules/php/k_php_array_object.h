/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _K_PHP_ARRAY_OBJECT_H_
#define _K_PHP_ARRAY_OBJECT_H_

#include "php_module.h"

namespace kroll
{
	class KPHPArrayObject : public KList
	{
		public:
		KPHPArrayObject(zval *list);
		virtual ~KPHPArrayObject();

		KValueRef Get(const char *name);
		void Set(const char *name, KValueRef value);
		virtual bool Equals(KObjectRef);
		SharedStringList GetPropertyNames();

		unsigned int Size();
		void Append(KValueRef value);
		virtual void SetAt(unsigned int index, KValueRef value);
		bool Remove(unsigned int index);
		KValueRef At(unsigned int index);

		zval* ToPHP();

		protected:
		zval *list;

		static void AddKrollValueToPHPArray(KValueRef value, zval *phpArray, const char *key);
		static void AddKrollValueToPHPArray(KValueRef value, zval *phpArray, unsigned int index);
		static void AddKrollValueToPHPArray(KValueRef value, zval *phpArray);
		DISALLOW_EVIL_CONSTRUCTORS(KPHPArrayObject);
	};
}
#endif
