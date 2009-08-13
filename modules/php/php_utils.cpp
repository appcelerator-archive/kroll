/**
* Appcelerator Kroll - licensed under the Apache Public License 2
* see LICENSE in the root folder for details on the license.
* Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
*/
#include "php_module.h"

namespace kroll
{
	SharedValue PhpUtils::ToKrollValue(zval *value)
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
				SharedKList l = new KPhpList(value);
				return Value::NewList(l);
			};
			case IS_RESOURCE: /*TODO: Implement*/
			case IS_OBJECT: /*TODO: Implement*/
			default: return Value::Null;
		}
	}

	zval* PhpUtils::ToPhpValue(SharedValue value)
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
			AutoPtr<KPhpList> pl = value->ToList().cast<KPhpList>();
			if (!pl.isNull())
				return pl->ToPhp();
			else
				return PhpUtils::KListToPhpValue(value);
		}

		return retval;
	}

	zval* PhpUtils::KListToPhpValue(SharedValue value)
	{
		zval* phpArray;
		MAKE_STD_ZVAL(phpArray);
		array_init(phpArray);

		/*TODO: Flesh out with full implementation.*/

		return phpArray;
	}

	void PhpUtils::AddKrollValueToPhpArray(SharedValue value, zval *phpArray, const char *key)
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
			AutoPtr<KPhpList> pl = value->ToList().cast<KPhpList>();
			if (!pl.isNull())
				phpValue = pl->ToPhp();
			else
				phpValue = PhpUtils::KListToPhpValue(value);

			add_assoc_zval(phpArray, (char *) key, phpValue);
		}
	}

	void PhpUtils::AddKrollValueToPhpArray(SharedValue value, zval *phpArray, unsigned int index)
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
			AutoPtr<KPhpList> pl = value->ToList().cast<KPhpList>();
			if (!pl.isNull())
				phpValue = pl->ToPhp();
			else
				phpValue = PhpUtils::KListToPhpValue(value);

			add_index_zval(phpArray, (unsigned long) index, phpValue);
		}
	}

	void PhpUtils::AddKrollValueToPhpArray(SharedValue value, zval *phpArray)
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
			AutoPtr<KPhpList> pl = value->ToList().cast<KPhpList>();
			if (!pl.isNull())
				phpValue = pl->ToPhp();
			else
				phpValue = PhpUtils::KListToPhpValue(value);

			add_next_index_zval(phpArray, phpValue);
		}
	}
}
