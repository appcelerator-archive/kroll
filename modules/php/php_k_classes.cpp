/**
* Appcelerator Kroll - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#include "php_module.h"

namespace kroll
{
	// Private data and function declarations below here
	static zend_class_entry *PHPKObjectClassEntry;
	static zend_class_entry *PHPKMethodClassEntry;
	static zend_class_entry *PHPKListClassEntry;
	static zend_object_handlers PHPKObjectHandlers;

	static zend_object_value PHPKObjectCreateObject(zend_class_entry *ce TSRMLS_DC);
	static void PHPKObjectFreeStorage(void* zthis TSRMLS_DC);
	static zval* PHPKObjectReadProperty(zval* zthis, zval* property, int type TSRMLS_DC);
	static void PHPKObjectWriteProperty(zval* zthis, zval* property, zval* value TSRMLS_DC);
	static HashTable* PHPKObjectGetProperties(zval* zthis TSRMLS_DC);
	static void PHPKObjectUnsetProperty(zval* zthis, zval* property TSRMLS_DC);
	static int PHPKObjectHasProperty(zval* zthis, zval* property, int type TSRMLS_DC);
	static int PHPKObjectHasDimension(zval* zthis, zval* property, int type TSRMLS_DC);
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

	PHP_METHOD(PHPKObject, __call)
	{
		char* methodName;
		int methodNameLength;
		zval* zargs;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa",
			&methodName, &methodNameLength, &zargs) == FAILURE)
		{
			zend_throw_exception(zend_exception_get_default(TSRMLS_C),
				(char*) "Wrong arguments passed to __call", 666 TSRMLS_CC);
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
				(char*) error.c_str(), 666 TSRMLS_CC);
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
				kargs.push_back(PHPUtils::ToKrollValue(*parameter TSRMLS_CC));
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
				(char*) e.AsString().c_str(), 666 TSRMLS_CC);
			RETVAL_NULL();
			return;
		}
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

	void PHPKObjectFreeStorage(void *zthis TSRMLS_DC)
	{
		PHPKObject* phpkobject = static_cast<PHPKObject*>(zthis);
		phpkobject->kvalue = 0;

		zend_object_std_dtor(&phpkobject->std TSRMLS_CC);
		efree(zthis);
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
				(char*) e.AsString().c_str(), 666 TSRMLS_CC);
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
		SharedValue krollValue = PHPUtils::ToKrollValue(value TSRMLS_CC);

		try
		{
			kobject->Set(propertyName.c_str(), krollValue);
		}
		catch (ValueException& e)
		{
			zend_throw_exception(
				zend_exception_get_default(TSRMLS_C), (char*) e.AsString().c_str(), 666 TSRMLS_CC);
		}
	}

	HashTable* PHPKObjectGetProperties(zval *zthis TSRMLS_DC)
	{
		PHPKObject *kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(zthis TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();

		try
		{
			SharedStringList propertyNames = kobject->GetPropertyNames();
			HashTable *properties;
			ALLOC_HASHTABLE(properties);
			zend_hash_init(properties, propertyNames->size(), NULL, ZVAL_PTR_DTOR, 0);

			for (size_t i = 0; i < propertyNames->size(); i++)
			{
				const char *key = propertyNames->at(i)->c_str();
				SharedValue value = kobject->Get(key);
				zval* zvalue = PHPUtils::ToPHPValue(value);
				zend_hash_add(properties, (char *)key, strlen(key)+1, &zvalue, sizeof(zval*), NULL);
			}
			return properties;
		}
		catch (ValueException& e)
		{
			zend_throw_exception(zend_exception_get_default(TSRMLS_C),
				(char *) e.AsString().c_str(), 666 TSRMLS_CC);
		}

		return 0;
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
				zend_exception_get_default(TSRMLS_C), (char*) e.AsString().c_str(), 666 TSRMLS_CC);
		}
	}

	namespace PHPUtils
	{
		void InitializePHPKrollClasses()
		{
			zend_class_entry ce;
			TSRMLS_FETCH();
			// Initialize the class entry for our classes
			INIT_CLASS_ENTRY(ce, "KObject", PHPKObjectMethods);
			PHPKObjectClassEntry = zend_register_internal_class(&ce TSRMLS_CC);
			PHPKObjectClassEntry->create_object = PHPKObjectCreateObject;

			// Create our custom handlers table to override the
			// default behaviour of our PHP objects.
			PHPKObjectHandlers = *zend_get_std_object_handlers();
			PHPKObjectHandlers.read_property = PHPKObjectReadProperty;
			PHPKObjectHandlers.write_property = PHPKObjectWriteProperty;
			PHPKObjectHandlers.get_properties = PHPKObjectGetProperties;
			PHPKObjectHandlers.read_dimension = PHPKObjectReadProperty;
			PHPKObjectHandlers.unset_property = PHPKObjectUnsetProperty;
			PHPKObjectHandlers.unset_dimension = PHPKObjectUnsetProperty;
			PHPKObjectHandlers.write_dimension = PHPKObjectWriteProperty;
			PHPKObjectHandlers.has_property = PHPKObjectHasProperty;
			PHPKObjectHandlers.has_dimension = PHPKObjectHasDimension;
		}
	}

	void KObjectToKPHPObject(SharedValue objectValue, zval** returnValue)
	{
		// Initialize our object with our pre-defined KObject class entry.
		TSRMLS_FETCH();
		object_init_ex(*returnValue, PHPKObjectClassEntry);

		// Place the KValue into the internal struct.
		PHPKObject* internal = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(*returnValue TSRMLS_CC));
		internal->kvalue = objectValue;
	}
}
