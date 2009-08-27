/**
* Appcelerator Kroll - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#include "php_module.h"
#include <spl/spl_array.h>

#define GET_MY_KLIST() \
	reinterpret_cast<PHPKObject*>( \
	 zend_object_store_get_object(getThis() TSRMLS_CC))->kvalue->ToList()

namespace kroll
{
	zend_class_entry *PHPKObjectClassEntry = NULL;
	zend_class_entry *PHPKMethodClassEntry = NULL;
	zend_class_entry *PHPKListClassEntry = NULL;
	zend_object_handlers PHPKObjectHandlers;
	zend_object_handlers PHPKMethodHandlers;
	zend_object_handlers PHPKListHandlers;

	// Private data and function declarations below here
	static zend_object_value PHPKObjectCreateObject(zend_class_entry *ce TSRMLS_DC);
	static zend_class_entry* PHPKObjectGetClassEntry(const zval* zthis TSRMLS_DC);
	static zend_class_entry* PHPKMethodGetClassEntry(const zval* zthis TSRMLS_DC);
	static zend_class_entry* PHPKListGetClassEntry(const zval* zthis TSRMLS_DC);
	static void PHPKObjectFreeStorage(void* zthis TSRMLS_DC);
	static zval* PHPKObjectReadProperty(zval* zthis, zval* property, int type TSRMLS_DC);
	static void PHPKObjectWriteProperty(zval* zthis, zval* property, zval* value TSRMLS_DC);
	static HashTable* PHPKObjectGetProperties(zval* zthis TSRMLS_DC);
	static void PHPKObjectUnsetProperty(zval* zthis, zval* property TSRMLS_DC);
	static int PHPKObjectHasProperty(zval* zthis, zval* property, int type TSRMLS_DC);
	static int PHPKObjectHasDimension(zval* zthis, zval* property, int type TSRMLS_DC);

	PHP_METHOD(PHPKObject, __call);
	PHP_METHOD(PHPKMethod, __invoke);
	PHP_METHOD(PHPKList, offsetExists);
	PHP_METHOD(PHPKList, offsetGet);
	PHP_METHOD(PHPKList, offsetUnset);
	PHP_METHOD(PHPKList, offsetSet);
	PHP_METHOD(PHPKList, count);
	PHP_METHOD(PHPKList, append);
	PHP_METHOD(PHPKList, getArrayCopy);
	PHP_METHOD(PHPKList, exchangeArray);

	static ZEND_FUNCTION(krollAddFunction);

	ZEND_BEGIN_ARG_INFO_EX(KrollAddFunctionArgInfo, 0, 0, 2)
		ZEND_ARG_INFO(0, object)
		ZEND_ARG_INFO(0, fname)
	ZEND_END_ARG_INFO()
	
	static const zend_function_entry PHPFunctions[] = {
		ZEND_FE(krollAddFunction, KrollAddFunctionArgInfo)
		{ NULL, NULL, NULL, NULL }
	};

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

	static function_entry PHPKMethodMethods[] =
	{
		PHP_ME(PHPKMethod, __invoke, NULL, ZEND_ACC_PUBLIC)
		PHP_ME(PHPKObject, __call, PHPKObjectCallArgInfo, ZEND_ACC_PUBLIC)
		{NULL, NULL, NULL}
	};

	ZEND_BEGIN_ARG_INFO_EX(PHPKListOffsetGetArgInfo, 0, 0, 1)
		ZEND_ARG_INFO(0, index)
	ZEND_END_ARG_INFO()
	ZEND_BEGIN_ARG_INFO_EX(PHPKListOffsetSetArgInfo, 0, 0, 2)
		ZEND_ARG_INFO(0, index)
		ZEND_ARG_INFO(0, newval)
	ZEND_END_ARG_INFO()
	ZEND_BEGIN_ARG_INFO(PHPKListAppendArgInfo, 0)
		ZEND_ARG_INFO(0, value)
	ZEND_END_ARG_INFO()
	ZEND_BEGIN_ARG_INFO(PHPKListExchangeArrayArgInfo, 0)
		ZEND_ARG_INFO(0, array)
	ZEND_END_ARG_INFO()
	ZEND_BEGIN_ARG_INFO(PHPKListVoidArgInfo, 0)
	ZEND_END_ARG_INFO()

	static function_entry PHPKListMethods[] =
	{
		PHP_ME(PHPKList, offsetExists, PHPKListOffsetGetArgInfo, ZEND_ACC_PUBLIC)
		PHP_ME(PHPKList, offsetGet, PHPKListOffsetGetArgInfo, ZEND_ACC_PUBLIC)
		PHP_ME(PHPKList, offsetSet, PHPKListOffsetSetArgInfo, ZEND_ACC_PUBLIC)
		PHP_ME(PHPKList, offsetUnset, PHPKListOffsetGetArgInfo, ZEND_ACC_PUBLIC)
		PHP_ME(PHPKList, count, PHPKListVoidArgInfo, ZEND_ACC_PUBLIC)
		PHP_ME(PHPKList, exchangeArray, PHPKListExchangeArrayArgInfo, ZEND_ACC_PUBLIC)
		PHP_ME(PHPKList, getArrayCopy, PHPKListVoidArgInfo, ZEND_ACC_PUBLIC)
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

	zend_object_value PHPKObjectCreateObject(zend_class_entry *classEntry TSRMLS_DC)
	{
		PHPKObject* intern;
		zend_object_value retval;

		// We're using a custom zend_object* (PHPKObject*) so we
		// need to do the things done by zend_objects_new manually.
		intern = (PHPKObject*) emalloc(sizeof(PHPKObject));
		memset(intern, 0, sizeof(PHPKObject));

		zend_object_std_init(&intern->std, classEntry TSRMLS_CC);

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
		if (classEntry == PHPKListClassEntry)
			retval.handlers = &PHPKListHandlers;
		else if (classEntry == PHPKMethodClassEntry)
			retval.handlers = &PHPKMethodHandlers;
		else
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
		std::string propertyName = PHPUtils::ZvalToPropertyName(property);

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

		std::string propertyName = PHPUtils::ZvalToPropertyName(property);
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
		std::string propertyName = PHPUtils::ZvalToPropertyName(property);

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
		std::string propertyName = PHPUtils::ZvalToPropertyName(property);

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

	zend_class_entry* PHPKObjectGetClassEntry(const zval* zthis TSRMLS_DC)
	{
		return PHPKObjectClassEntry;
	}

	zend_class_entry* PHPKMethodGetClassEntry(const zval* zthis TSRMLS_DC)
	{
		return PHPKMethodClassEntry;
	}

	zend_class_entry* PHPKListGetClassEntry(const zval* zthis TSRMLS_DC)
	{
		return PHPKListClassEntry;
	}

	void PHPKObjectUnsetProperty(zval* zthis, zval* property TSRMLS_DC)
	{
		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(zthis TSRMLS_CC));
		SharedKObject kobject = kthis->kvalue->ToObject();
		std::string propertyName = PHPUtils::ZvalToPropertyName(property);

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

	PHP_METHOD(PHPKMethod, __invoke)
	{
		PHPKObject* kthis = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(getThis() TSRMLS_CC));
		SharedKMethod kmethod = kthis->kvalue->ToMethod();

		zend_function *func = EG(current_execute_data)->function_state.function;
		zval*** arguments = (zval***) emalloc(sizeof(zval**) * ZEND_NUM_ARGS());

		if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), arguments) == FAILURE)
		{
			efree(arguments);
			zend_error(E_RECOVERABLE_ERROR, "Cannot get arguments for calling closure");
			RETVAL_FALSE;
			return;
		}

		ArgList kargs;
		for (int i = 0; i < ZEND_NUM_ARGS(); i++)
		{
			zval** zargValue = arguments[i];
			SharedValue argValue = PHPUtils::ToKrollValue(*zargValue TSRMLS_CC);
			kargs.push_back(argValue);
		}
		efree(arguments);

		// CAUTION: FRIGGIN SWEET METHOD INVOCATION COMING UP.
		try
		{
			SharedValue returnValue = kmethod->Call(kargs);
			PHPUtils::ToPHPValue(returnValue, &return_value);
		}
		catch (ValueException& e)
		{
			// TODO: Create an exception class that can hold a SharedValue.
			zend_throw_exception(zend_exception_get_default(TSRMLS_C),
				(char*) e.AsString().c_str(), 666 TSRMLS_CC);
			RETVAL_NULL();
			return;
		}
	}

	PHP_METHOD(PHPKList, offsetExists)
	{
		zval *index;
		if (zend_parse_parameters(ZEND_NUM_ARGS()
			TSRMLS_CC, "z", &index) == FAILURE) {
			return;
		}

		SharedKList klist(GET_MY_KLIST());
		std::string name(PHPUtils::ZvalToPropertyName(index));
		RETURN_BOOL((!name.empty() && klist->HasProperty(name.c_str())));
	}

	PHP_METHOD(PHPKList, offsetGet)
	{
		zval *index;
		if (zend_parse_parameters(ZEND_NUM_ARGS()
			TSRMLS_CC, "z", &index) == FAILURE)
		 {
			return;
			
		}

		SharedKList klist(GET_MY_KLIST());
		std::string name(PHPUtils::ZvalToPropertyName(index));
		SharedValue returnValue(klist->Get(name.c_str()));
		PHPUtils::ToPHPValue(returnValue, &return_value);
	}

	PHP_METHOD(PHPKList, offsetSet)
	{
		zval *zindexString, *zvalue;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz",
			&zindexString, &zvalue) == FAILURE)
		{
			return;
		}

		SharedKList klist(GET_MY_KLIST());
		std::string indexString(PHPUtils::ZvalToPropertyName(zindexString));
		SharedValue value(PHPUtils::ToKrollValue(zvalue TSRMLS_CC));

		int index = 0;
		if (KList::IsInt(indexString) &&
			((index = atoi(indexString.c_str())) >= 0))
		{
			klist->SetAt((unsigned int) index, value);
		}
		else
		{
			klist->Set(indexString.c_str(), value);
		}
	}

	PHP_METHOD(PHPKList, offsetUnset)
	{
		zval *zindex;
		if (zend_parse_parameters(ZEND_NUM_ARGS()
			TSRMLS_CC, "z", &index) == FAILURE) {
			return;
		}

		SharedKList klist(GET_MY_KLIST());
		std::string indexString(PHPUtils::ZvalToPropertyName(zindex));
		klist->Set(indexString.c_str(), Value::Undefined);
	}

	PHP_METHOD(PHPKList, count)
	{
		SharedKList klist(GET_MY_KLIST());
		SharedStringList propertyList = klist->GetPropertyNames();
		RETVAL_LONG(propertyList->size());
	}

	PHP_METHOD(PHPKList, append)
	{
		zval *zvalue;
		if (zend_parse_parameters(ZEND_NUM_ARGS()
			TSRMLS_CC, "z", &zvalue) == FAILURE)
		{
			return;
		}

		SharedKList klist(GET_MY_KLIST());
		SharedValue value(PHPUtils::ToKrollValue(zvalue TSRMLS_CC));
		klist->Append(value);
	} 

	PHP_METHOD(PHPKList, getArrayCopy)
	{
		SharedKList klist(GET_MY_KLIST());
		SharedStringList propertyList = klist->GetPropertyNames();

		array_init(return_value);
		for (size_t i = 0; i <= propertyList->size(); i++)
		{
			SharedString ss(propertyList->at(i));
			zval* newValue = PHPUtils::ToPHPValue(klist->Get(ss->c_str()));
			zend_hash_add(HASH_OF(return_value), ss->c_str(), strlen(ss->c_str()) - 1,
				&newValue, sizeof(zval*), NULL);
		}
	}

	PHP_METHOD(PHPKList, exchangeArray)
	{
		// TODO: Implement
	}

	ZEND_FUNCTION(krollAddFunction)
	{
		zval *phpWindowContext;
		char *fname;
		int fnameLength;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs",
			&phpWindowContext, &fname, &fnameLength) == FAILURE)
		{
			return;
		}
		
		PHPKObject* object = reinterpret_cast<PHPKObject*>(
			zend_object_store_get_object(phpWindowContext TSRMLS_CC));
			
		SharedKObject window = object->kvalue->ToObject();
		std::string functionName(fname, fnameLength);
		window->Set(functionName.c_str(), Value::NewMethod(new KPHPMethod(functionName.c_str())));
	}

	namespace PHPUtils
	{
		void InitializePHPKrollClasses()
		{
			TSRMLS_FETCH();

			// Initialize the class entry for our classes
			zend_class_entry kobjectClassEntry;
			INIT_CLASS_ENTRY(kobjectClassEntry, "PHPKObject", PHPKObjectMethods);
			PHPKObjectClassEntry = zend_register_internal_class(&kobjectClassEntry TSRMLS_CC);
			zend_class_entry kmethodClassEntry;
			INIT_CLASS_ENTRY(kmethodClassEntry, "PHPKMethod", PHPKMethodMethods);
			PHPKMethodClassEntry = zend_register_internal_class(&kmethodClassEntry TSRMLS_CC);
			zend_class_entry klistClassEntry;
			INIT_CLASS_ENTRY(klistClassEntry, "PHPKList", PHPKListMethods);
			PHPKListClassEntry = zend_register_internal_class_ex(
				&klistClassEntry, spl_ce_ArrayObject, "ArrayObject" TSRMLS_CC);

			// PHPKMethod has enough of the same behavior that we can use the same
			// handler table that PHPKObject uses. This may change in the future.
			PHPKObjectClassEntry->create_object = PHPKObjectCreateObject;
			PHPKMethodClassEntry->create_object = PHPKObjectCreateObject;
			PHPKListClassEntry->create_object = PHPKObjectCreateObject;

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
			PHPKObjectHandlers.get_class_entry = PHPKObjectGetClassEntry;

			PHPKListHandlers = PHPKObjectHandlers;
			PHPKListHandlers.get_class_entry = PHPKListGetClassEntry;

			// PHPKList mostly uses the standard handlers.
			PHPKMethodHandlers = *zend_get_std_object_handlers();
			PHPKMethodHandlers.get_class_entry = PHPKMethodGetClassEntry;

			// Initialize static functions
			zend_register_functions(NULL, PHPFunctions, NULL, MODULE_PERSISTENT TSRMLS_CC);
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

		void KMethodToKPHPMethod(SharedValue methodValue, zval** returnValue)
		{
			// Initialize our object with our pre-defined KObject class entry.
			TSRMLS_FETCH();
			object_init_ex(*returnValue, PHPKMethodClassEntry);

			// Place the KValue into the internal struct.
			PHPKObject* internal = reinterpret_cast<PHPKObject*>(
				zend_object_store_get_object(*returnValue TSRMLS_CC));
			internal->kvalue = methodValue;
		}

		void KListToKPHPArray(SharedValue listValue, zval** returnValue)
		{
			// Initialize our object with our pre-defined KObject class entry.
			TSRMLS_FETCH();
			object_init_ex(*returnValue, PHPKListClassEntry);

			// Place the KValue into the internal struct.
			PHPKObject* internal = reinterpret_cast<PHPKObject*>(
				zend_object_store_get_object(*returnValue TSRMLS_CC));
			internal->kvalue = listValue;
		}
	}
}
