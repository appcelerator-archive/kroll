/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "php_module.h"

namespace kroll {

	KPHPObject::KPHPObject(zval* object) :
		KObject("PHP.KPHPObject"),
		object(object)
	{
		zval_addref_p(object);
	}

	KPHPObject::~KPHPObject()
	{
		zval_delref_p(object);
	}

	void KPHPObject::Set(const char *name, KValueRef value)
	{
		// zend_update_property will call the write_property handler, but will
		// error out if the handler is NULL. add_property_zval_ex will try to
		// call the write_property handler without checking if it's NULL. Both
		// of these functions are exposed in the API with *NO* documentation.
		// and terribly misleading names.

		// That said, we'll do our own check for the write_property handler and
		// throw our own exception if it fails. This way we can handle the NULL
		// case, but not have to deal with zend_error handling.

		if (Z_OBJ_HANDLER_P(object, write_property))
		{
			zval* zvalue = PHPUtils::ToPHPValue(value);
			TSRMLS_FETCH();
			add_property_zval_ex(object, name, strlen(name)+1, zvalue TSRMLS_CC);
		}
		else
		{
			throw ValueException::FromFormat("Could not set property '%s': "
				"missing write_property handler", name);
		}
	}

	KValueRef KPHPObject::Get(const char *name)
	{
		zval** zPropertyPtr;
		unsigned int nameLength = strlen(name);
		TSRMLS_FETCH();

		// First try to get the property via the read_property handler.
		if (this->PropertyExists(name TSRMLS_CC) && Z_OBJ_HANDLER_P(object, read_property))
		{
			zval zname;
			ZVAL_STRINGL(&zname, name, nameLength, 0);

			// Use the read_property handler in the class.
			zval* zProperty = Z_OBJ_HANDLER_P(object, read_property)(
				object, &zname, 2 TSRMLS_CC);

			return PHPUtils::ToKrollValue(zProperty TSRMLS_CC);

		} // Next just try reading it from the properties hash.
		else if (zend_hash_find(Z_OBJPROP_P(object),
			name, nameLength + 1, (void**) &zPropertyPtr) != FAILURE)
		{
			return PHPUtils::ToKrollValue(*zPropertyPtr TSRMLS_CC);

		} // Check if the method exists on the object
		else if (this->MethodExists(name TSRMLS_CC))
		{
			return Value::NewMethod(new KPHPMethod(object, name));
		}
		else
		{
			return Value::Undefined;
		}
	}

	bool KPHPObject::Equals(KObjectRef other)
	{
		AutoPtr<KPHPObject> phpOther = other.cast<KPHPObject>();
		if (phpOther.isNull())
			return false;
		
		TSRMLS_FETCH();
		return PHPUtils::PHPObjectsEqual(this->ToPHP(), phpOther->ToPHP() TSRMLS_CC);
	}

	SharedString KPHPObject::DisplayString(int levels)
	{
		// Make a copy of our object zval* and then convert it to a string.
		zval* tempObject = NULL;
		ALLOC_ZVAL(tempObject);
		*tempObject = *object;
		INIT_PZVAL(tempObject);
		zval_copy_ctor(tempObject);

		convert_to_string(tempObject);
		return new std::string(Z_STRVAL_P(tempObject));
	}

	SharedStringList KPHPObject::GetPropertyNames()
	{
		TSRMLS_FETCH();
		SharedStringList filteredNames(new StringList());

		// If there is no get_properties handler, return. Why this would
		// happen, I have no idea -- from zend_builtin_functions.c:2363
		if (Z_OBJ_HT_P(object)->get_properties == NULL)
			return filteredNames;

		HashTable* properties = Z_OBJ_HT_P(object)->get_properties(object TSRMLS_CC);
		if (properties == NULL)
			return filteredNames;

		SharedStringList names(PHPUtils::GetHashKeys(properties));

		// Get the internal zend_object*.
		zend_object* internal = reinterpret_cast<zend_object*>(
			zend_object_store_get_object(object TSRMLS_CC));
		for (int i = 0; i < names->size(); i++)
		{
			std::string& name = *names->at(i);
			unsigned int nameLength = name.size();

			if (!zend_check_property_access(internal, (char*) name.c_str(),
				 nameLength-1 TSRMLS_CC) == SUCCESS)
				continue;

			char* unmangledPropertyName;
			char* className;
			zend_unmangle_property_name((char*) name.c_str(), nameLength-1, 
				&className, &unmangledPropertyName);
			filteredNames->push_back(new std::string(unmangledPropertyName));
		}

		SharedStringList methods(PHPUtils::GetClassMethods(Z_OBJCE_P(object) TSRMLS_CC));
		for (size_t i = 0; i < methods->size(); i++)
		{
			filteredNames->push_back(methods->at(i));
		}
		
		return filteredNames;
	}

	zval* KPHPObject::ToPHP()
	{
		return this->object;
	}

	bool KPHPObject::PropertyExists(const char* property TSRMLS_DC)
	{
		unsigned int propertyLength = strlen(property);
		zend_class_entry* classEntry = Z_OBJCE_P(object);

		// Check in the class entry for the property.
		ulong hashEntry = zend_get_hash_value(property, propertyLength + 1);
		zend_property_info* propertyInfo;
		if (SUCCESS == zend_hash_quick_find(&classEntry->properties_info,
			property, propertyLength + 1,
			hashEntry, (void **) &propertyInfo))
		{
			return propertyInfo->flags & ZEND_ACC_SHADOW;
		}

		// Check with the has_property handler in the class for the property.
		zval zPropertyName;
		ZVAL_STRINGL(&zPropertyName, property, propertyLength, 0);
		return (Z_OBJ_HANDLER_P(object, has_property) &&
			Z_OBJ_HANDLER_P(object, has_property)(object, &zPropertyName, 2 TSRMLS_CC));
	}
	
	bool KPHPObject::MethodExists(const char* methodName TSRMLS_DC)
	{
		unsigned int methodNameLength = strlen(methodName);
		char* lcMethodName = zend_str_tolower_dup(methodName, strlen(methodName));
		char *lcname;
		bool hasMethod = false;
		zend_class_entry* classEntry = Z_OBJCE_P(object);

		// First check the class "function" table.
		if (zend_hash_exists(&classEntry->function_table, lcMethodName, methodNameLength + 1))
		{
			hasMethod = true;
		}
		else if (Z_OBJ_HT_P(object)->get_method != NULL)
		{
			// This method exists if get_method returns non-null method pointer
			// which is not the standard __call method handler.
			union _zend_function *func = Z_OBJ_HT_P(object)->get_method(
				&object, (char*) methodName, methodNameLength TSRMLS_CC);

			if (func != NULL && (func->type != ZEND_INTERNAL_FUNCTION ||
				((zend_internal_function*)func)->handler == zend_std_call_user_call))
			{
				hasMethod = true;
			}
		}

		efree(lcMethodName);
		return hasMethod;
	}
}
