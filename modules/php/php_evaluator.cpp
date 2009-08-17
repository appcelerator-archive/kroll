/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "php_module.h"

namespace kroll
{
	SharedValue PHPEvaluator::Call(const ValueList& args)
	{
		if (args.size() != 3
			|| !args.at(1)->IsString()
			|| !args.at(2)->IsObject())
		{
			return Value::Undefined;
		}

		const char* code = args[1]->ToString();
		SharedKObject window_global = args.at(2)->ToObject();
		const char* name = "PHPEvaluator::Call";
		SharedValue kv = Value::Undefined;

		// Execute the PHP code
		TSRMLS_FETCH();
		zend_first_try {
			zval* return_value;

			// TODO: Need to implement error reporting here
			php_start_ob_buffer(NULL, 0, 1 TSRMLS_CC);
			zend_eval_string((char *) code, NULL, (char *) name TSRMLS_CC);
			php_ob_get_buffer(return_value TSRMLS_CC);
			php_end_ob_buffer(0, 0 TSRMLS_CC);

			SharedValue kv = PHPUtils::ToKrollValue(return_value);

			zval_dtor(return_value);
			FREE_ZVAL(return_value);
		} zend_end_try();

		return kv;
	}

	void PHPEvaluator::Set(const char *name, SharedValue value)
	{
	}

	SharedValue PHPEvaluator::Get(const char *name)
	{
		return Value::Undefined;
	}

	SharedStringList PHPEvaluator::GetPropertyNames()
	{
		return SharedStringList();
	}
}
