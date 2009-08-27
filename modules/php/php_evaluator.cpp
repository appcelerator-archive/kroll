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
		SharedKObject windowGlobal = args.at(2)->ToObject();
		const char* name = "<embedded PHP>";
		SharedValue kv = Value::Undefined;

		std::string contextName = CreateContextName();
		std::ostringstream codeString, callString;
		codeString << "function " << contextName << "() {\n";
		codeString << " global $Titanium, $window, $document;\n";
		codeString << code;
		codeString << " foreach (get_defined_vars() as $var=>$val) {\n";
		codeString << "  if ($var != 'Titanium' && $var != 'window' && $var != 'document') {\n";
		codeString << "    $window->$var = $val;\n";
		codeString << "  }\n";
		codeString << " }\n ";
		codeString << " $__fns = get_defined_functions();\n";
		codeString << " if (array_key_exists(\"user\", $__fns)) {\n";
		codeString << "  foreach($__fns[\"user\"] as $fname) {\n";
		codeString << "   if ($fname != \"" << contextName << "\" && !$window->$fname) {";
		codeString << "     krollAddFunction($window, $fname);\n";
		codeString << "   }\n";
		codeString << "  }\n";
		codeString << " }\n";
		
		//kroll_populate_context($window);\n";
		codeString << "};\n";
		codeString << contextName << "();";
		zend_first_try {
			/* This seems to be needed to make PHP actually give us errors at parse/compile time
			 * See: main/main.c line 969 */
			PG(during_request_startup) = 0;
			
			zval *windowValue = PHPUtils::ToPHPValue(args.at(2));
			ZEND_SET_SYMBOL(&EG(symbol_table), "window", windowValue);
			SharedValue document = windowGlobal->Get("document");
			zval *documentValue = PHPUtils::ToPHPValue(document);
			ZEND_SET_SYMBOL(&EG(symbol_table), "document", documentValue);
			
			zend_eval_string((char *) codeString.str().c_str(), NULL, (char *) name TSRMLS_CC);
		} zend_catch {
		} zend_end_try();

		return kv;
	}
	
	std::string PHPEvaluator::CreateContextName()
	{
		std::ostringstream contextName;
		contextName << "_kroll_context_" << rand();
		return contextName.str();
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
