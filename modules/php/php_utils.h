/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_TYPES_H_
#define _PHP_TYPES_H_

#include <typeinfo>
#include "php_module.h"

namespace kroll
{
	class PHPUtils
	{
	public:
		static SharedValue ToKrollValue(zval* value);
		static zval* ToPHPValue(SharedValue value);
		static zval* KListToPHPValue(SharedValue value);
		static void AddKrollValueToPHPArray(SharedValue value, zval *phpArray, const char *key);
		static void AddKrollValueToPHPArray(SharedValue value, zval *phpArray, unsigned int index);
		static void AddKrollValueToPHPArray(SharedValue value, zval *phpArray);

	private:
		PHPUtils() {}
		~PHPUtils () {}
	};
}

#endif
