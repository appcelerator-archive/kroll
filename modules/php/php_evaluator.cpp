/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "php_module.h"

namespace kroll
{
    SharedValue PhpEvaluator::Call(const ValueList& args)
    {
        if (args.size() != 3
            || !args.at(1)->IsString()
            || !args.at(2)->IsObject())
        {
            return Value::Undefined;
        }

        const char* code = args[1]->ToString();
		SharedKObject window_global = args.at(2)->ToObject();
        const char* name = "PhpEvaluator::Call";
		SharedValue kv = Value::Undefined;

        // Execute the PHP code
        zend_first_try {
            zval* return_value;

            // TODO: Need to implement error reporting here

            php_start_ob_buffer(NULL, 0, 1 TSRMLS_CC);
            zend_eval_string((char *) code, NULL, (char *) name TSRMLS_CC);
            php_ob_get_buffer(return_value TSRMLS_CC);
            php_end_ob_buffer(0, 0 TSRMLS_CC);

            SharedValue kv = PhpUtils::ToKrollValue(return_value);

            zval_dtor(return_value);
            FREE_ZVAL(return_value);
        } zend_end_try();

        return kv;
    }

	void PhpEvaluator::Set(const char *name, SharedValue value)
	{
	}

	SharedValue PhpEvaluator::Get(const char *name)
	{
		return Value::Undefined;
	}

	SharedStringList PhpEvaluator::GetPropertyNames()
	{
		return SharedStringList();
	}
}
