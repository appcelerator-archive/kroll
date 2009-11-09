/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_TYPES_H_
#define _PHP_TYPES_H_

#include "php_module.h"

namespace kroll
{
	typedef struct
	{
		zend_object std;
		KValueRef kvalue;
	} PHPKObject;
	extern zend_class_entry *PHPKObjectClassEntry;
	extern zend_class_entry *PHPKMethodClassEntry;
	extern zend_class_entry *PHPKListClassEntry;
	extern zend_object_handlers PHPKObjectHandlers;

	namespace PHPUtils
	{
		KValueRef ToKrollValue(zval* value TSRMLS_DC);
		zval* ToPHPValue(KValueRef value);
		void ToPHPValue(KValueRef value, zval** returnValue);
		std::string ZvalToPropertyName(zval* property);
		KListRef PHPArrayToKList(zval* array TSRMLS_DC,
			bool ignoreGlobals=false);
		KListRef PHPHashTableToKList(HashTable* hashtable TSRMLS_DC,
			 bool ignoreGlobals=false);
		SharedStringList GetHashKeys(HashTable* hash);
		void KObjectToKPHPObject(KValueRef objectValue, zval** returnValue);
		void KMethodToKPHPMethod(KValueRef methodValue, zval** returnValue);
		void KListToKPHPArray(KValueRef listValue, zval** returnValue);
		void InitializePHPKrollClasses();
		bool PHPObjectsEqual(zval* val1, zval* val2 TSRMLS_DC);
		int HashZvalCompareCallback(const zval** one, const zval** two TSRMLS_DC);
		SharedStringList GetClassMethods(zend_class_entry* ce TSRMLS_DC);
		KListRef GetClassVars(zend_class_entry* ce TSRMLS_DC);
		zend_function* GetGlobalFunction(const char *name TSRMLS_DC);
		void GenerateCaseMap(string code TSRMLS_DC);

		KObjectRef GetCurrentGlobalObject();
		void PushPHPSymbolsIntoGlobalObject(HashTable* symbolTable, KObjectRef global TSRMLS_DC);
		void PushGlobalObjectMembersIntoPHPSymbolTable(HashTable* symbolTable, KObjectRef global TSRMLS_DC);
		void SwapGlobalObject(KObjectRef newGlobal, HashTable* symbolTable TSRMLS_DC);
	}
}

#endif
