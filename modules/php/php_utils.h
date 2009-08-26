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
	extern zend_class_entry *PHPKObjectClassEntry;
	extern zend_class_entry *PHPKMethodClassEntry;
	extern zend_class_entry *PHPKListClassEntry;
	extern zend_object_handlers PHPKObjectHandlers;

	namespace PHPUtils
	{
		SharedValue ToKrollValue(zval* value TSRMLS_DC);
		zval* ToPHPValue(SharedValue value);
		void ToPHPValue(SharedValue value, zval** returnValue);
		std::string ZValToPropertyName(zval* property);
		SharedKList PHPArrayToStaticBoundList(zval* array TSRMLS_DC);
		SharedStringList GetHashKeys(HashTable *hash);
		void KObjectToKPHPObject(SharedValue objectValue, zval** returnValue);
		void KMethodToKPHPMethod(SharedValue methodValue, zval** returnValue);
		void InitializePHPKrollClasses();
		bool PHPObjectsEqual(zval* val1, zval* val2 TSRMLS_DC);
		SharedStringList GetClassMethods(zend_class_entry *ce TSRMLS_DC);
		SharedKList GetClassVars(zend_class_entry *ce TSRMLS_DC);
		zend_function* GetGlobalFunction(const char *name TSRMLS_DC);
		void PopulateContext(SharedKObject windowGlobal, HashTable *symbol_table TSRMLS_DC);
	}
}

#endif
