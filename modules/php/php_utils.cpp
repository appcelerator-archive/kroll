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

				list->Set(key, ToKrollValue(*value TSRMLS_CC));
			}

			return list;
		}
	}
}
