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
		KValueRef ToKrollValue(zval *value TSRMLS_DC)
		{
			KValueRef returnValue = Value::NewNull();
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
				// here and turn it into a KList.
				returnValue = Value::NewList(PHPArrayToKList(value TSRMLS_CC));
			}
			else if (IS_OBJECT == type)
			{
				if (HAS_CLASS_ENTRY(*value) &&
					Z_OBJCE_P(value) == PHPKObjectClassEntry ||
					Z_OBJCE_P(value) == PHPKMethodClassEntry)
				{
					PHPKObject* phpKObject = reinterpret_cast<PHPKObject*>(
						zend_object_store_get_object(value TSRMLS_CC));
					returnValue = phpKObject->kvalue;
				}
				else if (HAS_CLASS_ENTRY(*value) && Z_OBJCE_P(value) == zend_ce_closure)
				{
					returnValue = Value::NewMethod(new KPHPMethod(value, "__invoke"));
				}
				else
				{
					returnValue = Value::NewObject(new KPHPObject(value));
				}
			}
			else if (IS_RESOURCE == type)
			{
				// TODO: Implement
			}
			return returnValue;
		}

		zval* ToPHPValue(KValueRef value)
		{
			zval* returnValue;
			ALLOC_INIT_ZVAL(returnValue);
			ToPHPValue(value, &returnValue);
			return returnValue;
		}

		void ToPHPValue(KValueRef value, zval** returnValue)
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
				KListToKPHPArray(value, returnValue);
			}
			else
			{
				ZVAL_NULL(*returnValue);
			}
		}

		std::string ZvalToPropertyName(zval* phpPropertyName)
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

		KListRef PHPArrayToKList(zval* array TSRMLS_DC, bool ignoreGlobals)
		{
			HashTable* arrayHash = Z_ARRVAL_P(array);
			return PHPHashTableToKList(arrayHash TSRMLS_CC);
		}

		KListRef PHPHashTableToKList(HashTable* hashTable TSRMLS_DC, bool ignoreGlobals)
		{
			KListRef list = new StaticBoundList();

			for (zend_hash_internal_pointer_reset(hashTable);
				zend_hash_has_more_elements(hashTable) == SUCCESS;
				zend_hash_move_forward(hashTable))
			{

				char* key;
				unsigned int keyLength;
				unsigned long index;
				int type = zend_hash_get_current_key_ex(
					hashTable, &key, &keyLength, &index, 0, NULL);

				zval** value;
				if (zend_hash_get_current_data(hashTable, (void**) &value) == FAILURE)
					continue;

				if (type == HASH_KEY_IS_STRING)
				{
					// When we are processing the symbol table, we never want to copy
					// the GLOBALS table, because it will put us into an infinite loop.
					if (ignoreGlobals && !strcmp(key, "GLOBALS"))
						continue;

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

		int HashZvalCompareCallback(const zval **z1, const zval **z2 TSRMLS_DC)
		{
			zval result;

			if (compare_function(&result, (zval *) *z1, (zval *) *z2 TSRMLS_CC) == FAILURE)
			{
				return 1;
			}

			return Z_LVAL(result);
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
		
		/* (zend_builtin_functions.c, line 962 */
		static void add_class_vars(zend_class_entry *ce, HashTable *properties, zval *return_value TSRMLS_DC)
		{
			if (zend_hash_num_elements(properties) > 0) {
				HashPosition pos;
				zval **prop;

				zend_hash_internal_pointer_reset_ex(properties, &pos);
				while (zend_hash_get_current_data_ex(properties, (void **) &prop, &pos) == SUCCESS) {
					char *key, *class_name, *prop_name;
					uint key_len;
					ulong num_index;
					int prop_name_len = 0;
					zval *prop_copy;
					zend_property_info *property_info;
					zval zprop_name;

					zend_hash_get_current_key_ex(properties, &key, &key_len, &num_index, 0, &pos);
					zend_hash_move_forward_ex(properties, &pos);

					zend_unmangle_property_name(key, key_len-1, &class_name, &prop_name);
					prop_name_len = strlen(prop_name);

					ZVAL_STRINGL(&zprop_name, prop_name, prop_name_len, 0);
					property_info = zend_get_property_info(ce, &zprop_name, 1 TSRMLS_CC);

					if (!property_info || property_info == &EG(std_property_info)) {
						continue;
					}

					/* copy: enforce read only access */
					ALLOC_ZVAL(prop_copy);
					*prop_copy = **prop;
					zval_copy_ctor(prop_copy);
					INIT_PZVAL(prop_copy);

					/* this is necessary to make it able to work with default array
				* properties, returned to user */
					if (Z_TYPE_P(prop_copy) == IS_CONSTANT_ARRAY || (Z_TYPE_P(prop_copy) & IS_CONSTANT_TYPE_MASK) == IS_CONSTANT) {
						zval_update_constant(&prop_copy, 0 TSRMLS_CC);
					}

					add_assoc_zval(return_value, prop_name, prop_copy);
				}
			}
		}

		KListRef GetClassVars(zend_class_entry *ce TSRMLS_DC)
		{
			zval classVars;
			array_init(&classVars);
			zend_update_class_constants(ce TSRMLS_CC);
			add_class_vars(ce, &ce->default_properties, &classVars TSRMLS_CC);
			add_class_vars(ce, CE_STATIC_MEMBERS(ce), &classVars TSRMLS_CC);
			return PHPArrayToKList(&classVars TSRMLS_CC);
		}

		zend_function* GetGlobalFunction(const char *name TSRMLS_DC)
		{
			zend_function *function;
			if (zend_hash_find(EG(function_table), (char*)name, strlen(name)+1, (void **) &function) == SUCCESS)
			{
				Logger::Get("PHP")->Debug("Succeeded finding Global function: %s", name);
				return function;
			}
			
			Logger::Get("PHP")->Debug("Failed to find Global function: %s", name);
			return 0;
		}

		static KObjectRef currentPHPGlobal(0);
		KObjectRef GetCurrentGlobalObject()
		{
			return currentPHPGlobal;
		}

		void PushPHPSymbolsIntoGlobalObject(HashTable* symbolTable,
			KObjectRef global TSRMLS_DC)
		{
			// Push the variables from the given symbol table to the global object.
			if (!global.isNull())
			{
				KListRef symbols(PHPHashTableToKList(symbolTable TSRMLS_CC, true));
				SharedStringList keys(symbols->GetPropertyNames());
				for (size_t i = 0; i < keys->size(); i++)
				{
					const char* name = keys->at(i)->c_str();
					KValueRef newValue(symbols->Get(name));
					if (!newValue->Equals(global->Get(name)))
					{
						global->Set(name, newValue);
					}
				}
			}
		}

		void PushGlobalObjectMembersIntoPHPSymbolTable(HashTable* symbolTable,
			KObjectRef global TSRMLS_DC)
		{
			// Move all variables from the new global object into the PHP symbol table.
			if (!global.isNull())
			{
				SharedStringList keys(global->GetPropertyNames());
				for (size_t i = 0; i < keys->size(); i++)
				{
					const char* name = keys->at(i)->c_str();
					zval* newValue;
					MAKE_STD_ZVAL(newValue); 
					ToPHPValue(global->Get(name), &newValue);
					ZEND_SET_SYMBOL(symbolTable, (char*) name, newValue);
				}
			}
		}

		void SwapGlobalObject(KObjectRef newGlobal, HashTable* symbolTable TSRMLS_DC)
		{
			PushPHPSymbolsIntoGlobalObject(symbolTable, currentPHPGlobal TSRMLS_CC);
			zend_hash_clean(symbolTable);
			currentPHPGlobal = newGlobal;
			PushGlobalObjectMembersIntoPHPSymbolTable(symbolTable, newGlobal TSRMLS_CC);
		}

	}
}
