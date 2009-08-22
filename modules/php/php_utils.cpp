/**
* Appcelerator Kroll - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#include "php_module.h"

namespace kroll
{
	namespace PHPUtils
	{
		SharedValue ToKrollValue(zval *value TSRMLS_DC)
		{
			SharedValue returnValue = Value::NewNull();
			int type = Z_TYPE_P(value);

			if (IS_NULL == type)
			{
				returnValue = Value::Null;
			}
			else if (IS_BOOL == type)
			{
				returnValue = Value::NewBool(Z_BVAL_P(value));
			}
			else if (IS_LONG == type)
			{
				returnValue = Value::NewDouble(Z_LVAL_P(value));
			}
			else if (IS_DOUBLE == type)
			{
				returnValue = Value::NewDouble(Z_DVAL_P(value));
			}
			else if (IS_STRING == type)
			{
				returnValue = Value::NewString(Z_STRVAL_P(value));
			}
			else if (IS_ARRAY == type)
			{
				// PHP arrays are almost always passed by value, which means
				// they are all just copies of each other. To emulate this
				// behavior we might as well just make a copy of the array
				// here and turn it into a StaticBoundList.
				returnValue = Value::NewList(PHPArrayToStaticBoundList(value TSRMLS_CC));
			}
			else if (IS_OBJECT == type)
			{
				Logger::Get("PHP")->Debug("transforming PHP object to Kroll Value");
				if (HAS_CLASS_ENTRY(*value) && Z_OBJCE_P(value) == zend_ce_closure)
				{
					Logger::Get("PHP")->Debug("i'm a method");
					returnValue = Value::NewMethod(new KPHPMethod(value, "__invoke"));
				}
				else
				{
					if (HAS_CLASS_ENTRY(*value) && Z_OBJCE_P(value) == PHPKObjectClassEntry)
					{
						PHPKObject* phpKObject = reinterpret_cast<PHPKObject*>(
							zend_object_store_get_object(value TSRMLS_CC));
						returnValue = phpKObject->kvalue;
					}
					else
					{
						returnValue = Value::NewObject(new KPHPObject(value));
					}
				}
			}
			else if (IS_RESOURCE == type)
			{
				// TODO: Implement
			}
			return returnValue;
		}

		zval* ToPHPValue(SharedValue value)
		{
			zval *returnValue;
			ALLOC_INIT_ZVAL(returnValue);
			ToPHPValue(value, &returnValue);
			return returnValue;
		}

		void ToPHPValue(SharedValue value, zval** returnValue)
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
				// All numbers passing between Kroll and and PHP will be implicitly
				// converted into floating point. This could cause some PHP to
				// function incorrectly if it's doing strict type checking. We
				// need to clearly document this.
				ZVAL_DOUBLE(*returnValue, value->ToNumber());
			}
			else if (value->IsString())
			{
				const char* cstr = value->ToString();
				ZVAL_STRINGL(*returnValue, (char *) cstr, strlen(cstr), 1);
			}
			else if (value->IsObject())
			{
				KObjectToKPHPObject(value, returnValue);
			}
			else if (value->IsMethod())
			{
				KMethodToKPHPMethod(value, returnValue);
			}
			else if (value->IsList())
			{
				// TODO: Turn this list into a ArrayObject-style object
			}
			else
			{
				ZVAL_NULL(*returnValue);
			}
		}

		std::string ZValToPropertyName(zval* phpPropertyName)
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

		SharedKList PHPArrayToStaticBoundList(zval* array TSRMLS_DC)
		{
			SharedKList list = new StaticBoundList();

			HashTable *arrayHash = Z_ARRVAL_P(array);
			for (zend_hash_internal_pointer_reset(arrayHash);
				zend_hash_has_more_elements(arrayHash) == SUCCESS;
				zend_hash_move_forward(arrayHash))
			{

				char* key;
				unsigned int keyLength;
				unsigned long index;
				int type = zend_hash_get_current_key_ex(
					arrayHash, &key, &keyLength, &index, 0, NULL);

				zval** value;
				if (zend_hash_get_current_data(arrayHash, (void**) &value) == FAILURE)
					continue;

				if (type == HASH_KEY_IS_STRING)
				{
					list->Set(key, ToKrollValue(*value TSRMLS_CC));
				}
				else // Numeric key
				{
					list->SetAt(index, ToKrollValue(*value TSRMLS_CC));
				}
			}

			return list;
		}

		SharedStringList GetHashKeys(HashTable *hash)
		{
			SharedStringList keys(new StringList());
			HashPosition position;

			for (zend_hash_internal_pointer_reset_ex(hash, &position);
				zend_hash_has_more_elements_ex(hash, &position) == SUCCESS;
				zend_hash_move_forward_ex(hash, &position))
			{

				char *key;
				unsigned int keyLength;
				unsigned long index;

				zend_hash_get_current_key_ex(hash, &key, 
					&keyLength, &index, 0, &position);
				keys->push_back(new std::string(key));
			}

			return keys;
		}

		bool PHPObjectsEqual(zval* val1, zval* val2 TSRMLS_DC)
		{
			zval result;
			ZVAL_LONG(&result, 1);
			zend_compare_objects(&result, val1, val2 TSRMLS_CC);
			return Z_LVAL_P(&result) == 0;
		}
		
		SharedStringList GetClassMethods(zend_class_entry *ce TSRMLS_DC)
		{
			/* copied from internal impl of get_class_methods, (zend_builtin_functions.c, line 1062)
			 * this doesn't work if we just pass a user defined class name in.
			 * could be scope related? */
			HashPosition pos;
			zend_function *mptr;
			SharedStringList methods(new StringList());
			zend_hash_internal_pointer_reset_ex(&ce->function_table, &pos);

			while (zend_hash_get_current_data_ex(&ce->function_table, (void **) &mptr, &pos) == SUCCESS)
			{
				if ((mptr->common.fn_flags & ZEND_ACC_PUBLIC)
					|| (EG(scope) &&
						(((mptr->common.fn_flags & ZEND_ACC_PROTECTED) &&
							zend_check_protected(mptr->common.scope, EG(scope)))
						|| ((mptr->common.fn_flags & ZEND_ACC_PRIVATE) &&
							EG(scope) == mptr->common.scope))))
				{
					char *key;
					uint key_len;
					ulong num_index;
					uint len = strlen(mptr->common.function_name);

					/* Do not display old-style inherited constructors */
					if ((mptr->common.fn_flags & ZEND_ACC_CTOR) == 0 ||
						mptr->common.scope == ce ||
						zend_hash_get_current_key_ex(&ce->function_table, &key, &key_len, &num_index, 0, &pos) != HASH_KEY_IS_STRING ||
						zend_binary_strcasecmp(key, key_len-1, mptr->common.function_name, len) == 0)
					{
						methods->push_back(new std::string(mptr->common.function_name));
					}
				}
			
				zend_hash_move_forward_ex(&ce->function_table, &pos);
			}
			
			return methods;
		}
	}
}
