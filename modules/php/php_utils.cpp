/**
* Appcelerator Kroll - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#include "php_module.h"

namespace kroll
{
	SharedValue PHPUtils::ToKrollValue(zval *value)
	{
		switch (Z_TYPE_P(value))
		{
			case IS_NULL: return Value::Null;
			case IS_BOOL: return Value::NewBool(Z_BVAL_P(value));
			case IS_LONG: return Value::NewInt((int) Z_LVAL_P(value));
			case IS_DOUBLE: return Value::NewDouble(Z_DVAL_P(value));
			case IS_STRING:
			{
				std::string s = Z_STRVAL_P(value);
				return Value::NewString(s);
			};
			case IS_ARRAY:
			{
				SharedKList l = new KPHPList(value);
				return Value::NewList(l);
			};
			case IS_RESOURCE: /*TODO: Implement*/
			case IS_OBJECT: /*TODO: Implement*/
			default: return Value::Null;
		}
	}

	zval* PHPUtils::ToPHPValue(SharedValue value)
	{
		zval *retval;
		ALLOC_INIT_ZVAL(retval);

		if (value->IsNull() || value->IsUndefined())
		{
			ZVAL_NULL(retval);
		}
		else if (value->IsBool())
		{
			if (value->ToBool())
			{
				ZVAL_TRUE(retval);
			}
			else
			{
				ZVAL_FALSE(retval);
			}
		}
		else if (value->IsNumber())
		{
			/* No way to check whether the number is an
			   integer or a double here. All Kroll numbers
			   are doubles, so return a double. This could
			   cause some PHP to function incorrectly if it's
			   doing strict type checking. */
			ZVAL_DOUBLE(retval, value->ToNumber());
		}
		else if (value->IsString())
		{
			ZVAL_STRINGL(retval, (char *) value->ToString(), strlen(value->ToString()), 1);
		}
		else if (value->IsObject())
		{
			/*TODO: Implement*/
		}
		else if (value->IsMethod())
		{
			/*TODO: Implement*/
		}
		else if (value->IsList())
		{
			AutoPtr<KPHPList> pl = value->ToList().cast<KPHPList>();
			if (!pl.isNull())
				return pl->ToPHP();
			else
				return PHPUtils::KListToPHPValue(value);
		}

		return retval;
	}

	zval* PHPUtils::KListToPHPValue(SharedValue value)
	{
		zval* phpArray;
		MAKE_STD_ZVAL(phpArray);
		array_init(phpArray);

		/*TODO: Flesh out with full implementation.*/

		return phpArray;
	}

	void PHPUtils::AddKrollValueToPHPArray(SharedValue value, zval *phpArray, const char *key)
	{
		if (value->IsNull() || value->IsUndefined())
		{
			add_assoc_null(phpArray, (char *) key);
		}
		else if (value->IsBool())
		{
			if (value->ToBool())
				add_assoc_bool(phpArray, (char *) key, 1);
			else
				add_assoc_bool(phpArray, (char *) key, 0);
		}
		else if (value->IsNumber())
		{
			/* No way to check whether the number is an
			   integer or a double here. All Kroll numbers
			   are doubles, so return a double. This could
			   cause some PHP to function incorrectly if it's
			   doing strict type checking. */
			add_assoc_double(phpArray, (char *) key, value->ToNumber());
		}
		else if (value->IsString())
		{
			add_assoc_stringl(phpArray, (char *) key, (char *) value->ToString(), strlen(value->ToString()), 1);
		}
		else if (value->IsObject())
		{
			/*TODO: Implement*/
		}
		else if (value->IsMethod())
		{
			/*TODO: Implement*/
		}
		else if (value->IsList())
		{
			zval *phpValue;
			AutoPtr<KPHPList> pl = value->ToList().cast<KPHPList>();
			if (!pl.isNull())
				phpValue = pl->ToPHP();
			else
				phpValue = PHPUtils::KListToPHPValue(value);

			add_assoc_zval(phpArray, (char *) key, phpValue);
		}
	}

	void PHPUtils::AddKrollValueToPHPArray(SharedValue value, zval *phpArray, unsigned int index)
	{
		if (value->IsNull() || value->IsUndefined())
		{
			add_index_null(phpArray, (unsigned long) index);
		}
		else if (value->IsBool())
		{
			if (value->ToBool())
				add_index_bool(phpArray, (unsigned long) index, 1);
			else
				add_index_bool(phpArray, (unsigned long) index, 0);
		}
		else if (value->IsNumber())
		{
			/* No way to check whether the number is an
			   integer or a double here. All Kroll numbers
			   are doubles, so return a double. This could
			   cause some PHP to function incorrectly if it's
			   doing strict type checking. */
			add_index_double(phpArray, (unsigned long) index, value->ToNumber());
		}
		else if (value->IsString())
		{
			add_index_stringl(phpArray, (unsigned long) index, (char *) value->ToString(), strlen(value->ToString()), 1);
		}
		else if (value->IsObject())
		{
			/*TODO: Implement*/
		}
		else if (value->IsMethod())
		{
			/*TODO: Implement*/
		}
		else if (value->IsList())
		{
			zval *phpValue;
			AutoPtr<KPHPList> pl = value->ToList().cast<KPHPList>();
			if (!pl.isNull())
				phpValue = pl->ToPHP();
			else
				phpValue = PHPUtils::KListToPHPValue(value);

			add_index_zval(phpArray, (unsigned long) index, phpValue);
		}
	}

	void PHPUtils::AddKrollValueToPHPArray(SharedValue value, zval *phpArray)
	{
		if (value->IsNull() || value->IsUndefined())
		{
			add_next_index_null(phpArray);
		}
		else if (value->IsBool())
		{
			if (value->ToBool())
				add_next_index_bool(phpArray, 1);
			else
				add_next_index_bool(phpArray, 0);
		}
		else if (value->IsNumber())
		{
			/* No way to check whether the number is an
			   integer or a double here. All Kroll numbers
			   are doubles, so return a double. This could
			   cause some PHP to function incorrectly if it's
			   doing strict type checking. */
			add_next_index_double(phpArray, value->ToNumber());
		}
		else if (value->IsString())
		{
			add_next_index_stringl(phpArray, (char *) value->ToString(), strlen(value->ToString()), 1);
		}
		else if (value->IsObject())
		{
			/*TODO: Implement*/
		}
		else if (value->IsMethod())
		{
			/*TODO: Implement*/
		}
		else if (value->IsList())
		{
			zval *phpValue;
			AutoPtr<KPHPList> pl = value->ToList().cast<KPHPList>();
			if (!pl.isNull())
				phpValue = pl->ToPHP();
			else
				phpValue = PHPUtils::KListToPHPValue(value);

			add_next_index_zval(phpArray, phpValue);
		}
	}

	// These are the class entries for all our Kroll objects in PHP
	zend_class_entry *PHPKObjectClassEntry;
	zend_class_entry *PHPKMethodClassEntry;
	zend_class_entry *PHPKListClassEntry;

	// These function declaration correspond to our PHPKObject handlers.
	zend_object_value PHPKObjectCreateObject(zend_class_entry *ce TSRMLS_DC);
	static void PHPKObjectFreeStorage(void *object TSRMLS_DC);
	zval* PHPKObjectReadProperty(zval* object, zval* property, int type TSRMLS_DC);
	void PHPKObjectWriteProperty(zval* object, zval* property, zval* value TSRMLS_DC);
	int PHPKObjectHasProperty(zval* object, zval* property, int chk_type TSRMLS_DC);
	int PHPKObjectHasDimension(zval* object, zval* property, int chk_type TSRMLS_DC);

	// This is our class "function" table. It's empty, because right now
	// we just override the class handlers which operate at a lower level.
	static function_entry PHPKObjectMethods[] =
	{
		{NULL, NULL, NULL}
	};

	typedef struct {
		zend_object std;
		SharedValue kvalue;
	} PHPKObject;

	// This structure keeps track of the custom handlers
	static zend_object_handlers PHPKObjectHandlers;

	PHP_MINIT_FUNCTION(InitializePHPKrollClasses)
	{
		zend_class_entry ce;

		// Initialize the class entry for our classes
		INIT_CLASS_ENTRY(ce, "KObject", PHPKObjectMethods);
		PHPKObjectClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
		PHPKObjectClassEntry->create_object = PHPKObjectCreateObject;

		// Create our custom handlers table to override the
		// default behaviour of our PHP objects.
		PHPKObjectHandlers = *zend_get_std_object_handlers();
		PHPKObjectHandlers.read_property = PHPKObjectReadProperty;
		PHPKObjectHandlers.write_property = PHPKObjectWriteProperty;
		PHPKObjectHandlers.read_dimension = PHPKObjectReadProperty;
		PHPKObjectHandlers.write_dimension = PHPKObjectWriteProperty;
		PHPKObjectHandlers.has_property = PHPKObjectHasProperty;;
		PHPKObjectHandlers.has_dimension = PHPKObjectHasDimension;;

		return SUCCESS;
	}

	zend_object_value PHPKObjectCreateObject(zend_class_entry *ce TSRMLS_DC)
	{
		PHPKObject* intern;
		zend_object_value retval;

		// We're using a custom zend_object* (PHPKObject*) so we
		// need to do the things done by zend_objects_new manually.
		intern = (PHPKObject*) emalloc(sizeof(PHPKObject));
		memset(intern, 0, sizeof(PHPKObject));

		zend_object_std_init(&intern->std, ce TSRMLS_CC);

		// We don't have any default properties in our object
		// so just start out with a blank properties hash.
		ALLOC_HASHTABLE(intern->std.properties);
		zend_hash_init(intern->std.properties,
			0, NULL, ZVAL_PTR_DTOR, 0);

		// Use the standard object destructor, but we want to use a
		// custom memory free so that we can deference the internal
		// Kroll value.
		retval.handle = zend_objects_store_put(intern,
			(zend_objects_store_dtor_t) zend_objects_destroy_object,
			(zend_objects_free_object_storage_t) PHPKObjectFreeStorage,
			NULL TSRMLS_CC);

		// Use our special handlers for doing common object operations.
		retval.handlers = &PHPKObjectHandlers;
		return retval;
	}

	static void PHPKObjectFreeStorage(void *object TSRMLS_DC)
	{
		PHPKObject* phpkobject = static_cast<PHPKObject*>(object);
		phpkobject->kvalue = 0;

		zend_object_std_dtor(&phpkobject->std TSRMLS_CC);
		efree(object);
	}

	zval* PHPKObjectReadProperty(zval* object, zval* property, int type TSRMLS_DC)
	{
		return NULL;
		// Future implementation:
		//SharedKObject kobject = static_cast<PHPKObject*>(object)->kvalue->ToObject();
		//std::string propertyName = PHPUtils::ZvalToString(property);

		//try
		//{
		//	SharedValue value = kobject->Get(propertyName);
		//	return ToPHPValue(value);
		//}
		//catch (ValueException& e)
		//{
		//	// TODO: Convert to PHP exception
		//}
	}

	void PHPKObjectWriteProperty(zval* object, zval* property, zval* value TSRMLS_DC)
	{
		// Future implementation:
		//SharedKObject kobject = static_cast<PHPKObject*>(object)->kvalue->ToObject();
		//std::string propertyName = PHPUtils::ZvalToString(property);
		//SharedValue krollValue = PHPUtils::ToKrollValue(value);

		//try
		//{
		//	kobject->Set(propertyName, krollValue);
		//}
		//catch (ValueException& e)
		//{
		//	// TODO: Convert to PHP exception
		//}
	}

	int PHPKObjectHasProperty(zval* object, zval* property, int chk_type TSRMLS_DC)
	{
		return 0;
	}

	int PHPKObjectHasDimension(zval* object, zval* property, int chk_type TSRMLS_DC)
	{
		return 0;
	}
}
