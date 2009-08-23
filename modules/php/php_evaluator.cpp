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
		TSRMLS_FETCH();
		
		if (args.size() != 3
			|| !args.at(1)->IsString()
			|| !args.at(2)->IsObject())
		{
			return Value::Undefined;
		}

		const char* code = args[1]->ToString();
		SharedKObject window_global = args.at(2)->ToObject();
		const char* name = "<embedded PHP>";
		SharedValue kv = Value::Undefined;

		// Execute the PHP code
		zend_first_try {
			//zval* return_value;

			// TODO: Need to implement error reporting here
			// Output buffering should be used in templating logic, not runtime evaluation (right?)
			//php_start_ob_buffer(NULL, 0, 1 TSRMLS_CC);
			//php_ob_get_buffer(return_value TSRMLS_CC);
			//php_end_ob_buffer(0, 0 TSRMLS_CC);

			//SharedValue kv = PHPUtils::ToKrollValue(return_value TSRMLS_CC);

			//zval_dtor(return_value);
			//FREE_ZVAL(return_value);
			
			/* This seems to be needed to make PHP actually give us errors at parse/compile time
			 * See: main/main.c line 969 */
			PG(during_request_startup) = 0;
			
			zval *windowValue = PHPUtils::ToPHPValue(args.at(2));
			ZEND_SET_SYMBOL(&EG(symbol_table), "window", windowValue);
			zval *documentValue = PHPUtils::ToPHPValue(args.at(2)->ToObject()->Get("document"));
			ZEND_SET_SYMBOL(&EG(symbol_table), "document", documentValue);
			
			zend_eval_string((char *) code, NULL, (char *) name TSRMLS_CC);
		} zend_catch {
			
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
