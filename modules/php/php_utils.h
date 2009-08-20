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
	typedef struct
	{
		zend_object std;
		SharedValue kvalue;
	} PHPKObject;

	namespace PHPUtils
	{
		SharedValue ToKrollValue(zval* value TSRMLS_DC);
		zval* ToPHPValue(SharedValue value);
		void ToPHPValue(SharedValue value, zval** returnValue);
		std::string ZValToPropertyName(zval* property);
		void KObjectToKPHPObject(SharedValue objectValue, zval** returnValue);
		void InitializePHPKrollClasses();
	}
}

#endif
