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
		zval *returnValue;
		ALLOC_INIT_ZVAL(returnValue);
		ToPHPValue(value, &returnValue);
	}

	void PHPUtils::ToPHPValue(SharedValue value, zval** returnValue)
	{
		if (value->IsNull() || value->IsUndefined())
		{
			ZVAL_NULL(*returnValue);
		}
		else if (value->IsBool())
		{
			if (value->ToBool())
			{
				ZVAL_TRUE(*returnValue);
			}
			else
			{
				ZVAL_FALSE(*returnValue);
			}
		}
		else if (value->IsNumber())
		{
			/* No way to check whether the number is an
			   integer or a double here. All Kroll numbers
			   are doubles, so return a double. This could
			   cause some PHP to function incorrectly if it's
			   doing strict type checking. */
			ZVAL_DOUBLE(*returnValue, value->ToNumber());
		}
		else if (value->IsString())
		{
			ZVAL_STRINGL(*returnValue, (char *) value->ToString(), strlen(value->ToString()), 1);
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
			// TODO: We need to figure out how to put this object
			// into the incomming zval*.
			// zval* list = NULL;

			// AutoPtr<KPHPList> pl = value->ToList().cast<KPHPList>();
			// if (!pl.isNull())
			// 	list = pl->ToPHP();
			// else
			// 	list = PHPUtils::KListToPHPValue(value);

			// ZVAL_RESOURCE(*returnValue, list);
		}
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

	std::string PHPUtils::ZValToPropertyName(zval* phpPropertyName)
	{
		// This will destroy the original value.
		convert_to_string(phpPropertyName);

		if (IS_STRING == Z_TYPE_P(phpPropertyName))
		{
			return Z_STRVAL_P(phpPropertyName);
		}
		else
		{
			throw ValueException::FromString(
				"Could not convert property name to string.");
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
	void PHPKObjectUnsetProperty(zval* object, zval* property TSRMLS_DC);
	int PHPKObjectHasProperty(zval* object, zval* property, int chk_type TSRMLS_DC);
	int PHPKObjectHasDimension(zval* object, zval* property, int chk_type TSRMLS_DC);
	PHP_METHOD(PHPKObject, __call);

	// This is our class "function" table. Right now we only implement
	// __call, because that seems to be preferred over the handler version.
	ZEND_BEGIN_ARG_INFO_EX(PHPKObjectCallArgInfo, 0, 0, 2)
	ZEND_ARG_INFO(0, methodName)
	ZEND_ARG_INFO(0, arguments)
	ZEND_END_ARG_INFO()

	static function_entry PHPKObjectMethods[] =
	{
		PHP_ME(PHPKObject, __call, PHPKObjectCallArgInfo, ZEND_ACC_PUBLIC)
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
		PHPKObjectHandlers.unset_property = PHPKObjectUnsetProperty;
		PHPKObjectHandlers.unset_dimension = PHPKObjectUnsetProperty;
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

	zval* PHPKObjectReadProperty(zval* zthis, zval* property, int type TSRMLS_DC)
	{
		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(zthis TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();
		std::string propertyName = PHPUtils::ZValToPropertyName(property);

		try
		{
			SharedValue value = kobject->Get(propertyName.c_str());
			return PHPUtils::ToPHPValue(value);
		}
		catch (ValueException& e)
		{
			zend_throw_exception(zend_exception_get_default(TSRMLS_C), 
				(char*) e.AsString().c_str(), 666);
			zval* retval = NULL;
			ZVAL_NULL(retval);
			return retval;
		}
	}

	void PHPKObjectWriteProperty(zval* zthis, zval* property, zval* value TSRMLS_DC)
	{
		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(zthis TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();

		std::string propertyName = PHPUtils::ZValToPropertyName(property);
		SharedValue krollValue = PHPUtils::ToKrollValue(value);

		try
		{
			kobject->Set(propertyName.c_str(), krollValue);
		}
		catch (ValueException& e)
		{
			zend_throw_exception(
				zend_exception_get_default(TSRMLS_C), (char*) e.AsString().c_str(), 666);
		}
	}

	/* Extending and Embedding PHP pg. 153
	 * When isset() is called against an object property, this handler is invoked.
	 * By default the standard handler will check for the property named by
	 * 'property', if it's not found and -- as of PHP 4.1.0 -- if an __isset()
	 * method is defined it will call that. The checkType parameter will be one of
	 * three possible values. If the value is 2 the property need only exist to
	 * qualify as a success. If the checkType is 0, it must exist and be of any
	 * type except IS_NULL. If the value of checkType is 1, the value msut both
	 * eist and evaluate to a non-false value. Note: In PHP 4.0.x the meaning of
	 * checkType matched has_dimension's version of checkType (Martin: but not any
	 * longer!).
	 */
	int PHPKObjectHasProperty(zval* zthis, zval* property, int checkType TSRMLS_DC)
	{
		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(zthis TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();
		std::string propertyName = PHPUtils::ZValToPropertyName(property);

		if (checkType == 0)
		{
			SharedValue value = kobject->Get(propertyName.c_str());
			return !value->IsUndefined() && !value->IsNull();
		}
		else if (checkType == 1)
		{
			SharedValue value = kobject->Get(propertyName.c_str());
			zval* phpValue = PHPUtils::ToPHPValue(value);
			convert_to_boolean(phpValue);
			return Z_BVAL_P(phpValue);
		}
		else // Generally this should be checkType == 2
		{
			return kobject->HasProperty(propertyName.c_str());
		}
	}

	/* Extending and Embedding PHP pg. 154
	 * When isset() is called against an object that is being treated like an
	 * array, such as isset($obj['idx']), this handler is used. The standard
	 * handler, if the object implements the ArrayAccess interface, will call the
	 * offsetexists($idx) method first. If not found, it returns failure in the
	 * form of a 0. Otherwise, if checkType is 0 it returns true (1) immediately.
	 * A checkType of 1 indicates that it must also check that the value is
	 * non-false by invoking the object's offsetget($idx) method as well and
	 * examining the returned value.
	 */
	int PHPKObjectHasDimension(zval* zthis, zval* property, int checkType TSRMLS_DC)
	{
		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(zthis TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();
		std::string propertyName = PHPUtils::ZValToPropertyName(property);

		if (checkType == 0)
		{
			return kobject->HasProperty(propertyName.c_str());
		}
		else
		{
			if (!kobject->HasProperty(propertyName.c_str()))
			{
				return false;
			}
			else
			{
				SharedValue value = kobject->Get(propertyName.c_str());
				zval* phpValue = PHPUtils::ToPHPValue(value);
				convert_to_boolean(phpValue);
				return Z_BVAL_P(phpValue);
			}
		}
	}

	void PHPKObjectUnsetProperty(zval* zthis, zval* property TSRMLS_DC)
	{
		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(zthis TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();
		std::string propertyName = PHPUtils::ZValToPropertyName(property);

		try
		{
			kobject->Set(propertyName.c_str(), Value::Undefined);
		}
		catch (ValueException& e)
		{
			zend_throw_exception(
				zend_exception_get_default(TSRMLS_C), (char*) e.AsString().c_str(), 666);
		}
	}

	PHP_METHOD(PHPKObject, __call)
	{
		char* methodName;
		int methodNameLength;
		zval* zargs;

		if (!zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa",
			&methodName, &methodNameLength, &zargs) == FAILURE)
		{
			zend_throw_exception(zend_exception_get_default(TSRMLS_C),
				(char*) "Wrong arguments passed to __call", 666);
			RETVAL_NULL();
			return;
		}

		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(getThis() TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();
		SharedKMethod method = kobject->GetMethod(methodName, 0);

		// Find the method by its name.
		if (method.isNull())
		{
			std::string error("Could not find method named '");
			error.append(methodName);
			error.append("'");
			zend_throw_exception(zend_exception_get_default(TSRMLS_C),
				(char*) error.c_str(), 666);
			RETVAL_NULL();
			return;
		}

		// Pull out the arguments from the argument array.
		ArgList kargs;
		int numArgs = zend_hash_num_elements(Z_ARRVAL_P(zargs));
		if (numArgs > 0)
		{
			HashPosition position;
			zval** parameter;
			zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(zargs), &position);

			while (zend_hash_get_current_data_ex(Z_ARRVAL_P(zargs),
				(void **) &parameter, &position) == SUCCESS)
			{
				kargs.push_back(PHPUtils::ToKrollValue(*parameter));
				zend_hash_move_forward_ex(Z_ARRVAL_P(zargs), &position);
			}
		}

		// Do the method invocation.
		try
		{
			SharedValue returnValue = method->Call(kargs);
			PHPUtils::ToPHPValue(returnValue, &return_value);
		}
		catch (ValueException& e)
		{
			zend_throw_exception(zend_exception_get_default(TSRMLS_C), 
				(char*) e.AsString().c_str(), 666);
			RETVAL_NULL();
			return;
		}

	}
}
