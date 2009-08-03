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
			case IS_RESOURCE: /*TODO: Implement*/
			case IS_ARRAY: /*TODO: Implement*/
			case IS_OBJECT: /*TODO: Implement*/
			default: return Value::Null;
		}
	}
}
