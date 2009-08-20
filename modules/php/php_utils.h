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
		static void ToPHPValue(SharedValue value, zval** returnValue);
		static std::string ZValToPropertyName(zval* property);
		static void CreatePHPKObject(SharedValue objectValue, zval** returnValue);
		
		// These function declaration correspond to our PHPKObject handlers.
		static void InitializePHPKrollClasses();
		static zend_object_value PHPKObjectCreateObject(zend_class_entry *ce TSRMLS_DC);
		static void PHPKObjectFreeStorage(void *object TSRMLS_DC);
		static zval* PHPKObjectReadProperty(zval* object, zval* property, int type TSRMLS_DC);
		static void PHPKObjectWriteProperty(zval* object, zval* property, zval* value TSRMLS_DC);
		static HashTable* PHPUtils::PHPKObjectGetProperties(zval *zthis TSRMLS_DC);
		static void PHPKObjectUnsetProperty(zval* object, zval* property TSRMLS_DC);
		static int PHPKObjectHasProperty(zval* object, zval* property, int chk_type TSRMLS_DC);
		static int PHPKObjectHasDimension(zval* object, zval* property, int chk_type TSRMLS_DC);

		private:
		PHPUtils() {}
		~PHPUtils() {}
	};
}

#endif
