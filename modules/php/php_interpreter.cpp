/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "php_module.h"

#include <sstream>
#include <map>
#include <algorithm>
 
using namespace std;

namespace kroll {

PHPInterpreter::PHPInterpreter()
{
}

static string GetContextId(KObjectRef global)
{
	string contextId(global->GetString("__php_module_id__"));
	if (contextId.empty())
	{
		static int nextId = 0;
		contextId.append("__kroll__namespace__");
		contextId.append(KList::IntToChars(++nextId));
		global->SetString("__php_module_id__", contextId);
	}

	return contextId;
}

KValueRef PHPInterpreter::EvaluateFile(const char* filepath, KObjectRef context)
{
	static Poco::Mutex evaluatorMutex;
	Poco::Mutex::ScopedLock evaluatorLock(evaluatorMutex);

	TSRMLS_FETCH();

    // Read in source code
    std::string code(FileUtils::ReadFile(filepath));

	// Contexts must be the same for runs with the same global object.
	string contextId(GetContextId(context));
	PHPUtils::GenerateCaseMap(code TSRMLS_CC);

	std::ostringstream codeString;
	codeString << "namespace " << contextId << " {\n";
	codeString << code << "\n";
	codeString << "  $__fns = get_defined_functions();\n";
	codeString << "  if (array_key_exists(\"user\", $__fns)) {\n";
	codeString << "   foreach($__fns[\"user\"] as $fname) {\n";
	codeString << "    if (!$window->$fname) {";
	codeString << "      krollAddFunction($window, $fname);\n";
	codeString << "    }\n";
	codeString << "   }\n";
	codeString << "  }\n";
	codeString << "}\n";
	printf("%s\n", codeString.str().c_str());

	// This seems to be needed to make PHP actually give us errors
	// at parse/compile time -- see: main/main.c line 969
	PG(during_request_startup) = 0;

	KObjectRef previousGlobal(PHPUtils::GetCurrentGlobalObject());
	PHPUtils::SwapGlobalObject(context, &EG(symbol_table) TSRMLS_CC);

	zend_first_try
	{
		zend_eval_string((char *) codeString.str().c_str(), NULL, 
			(char *) filepath TSRMLS_CC);
	}
	zend_catch
	{
		Logger::Get("PHP")->Error("Evaluation of script failed");
	}
	zend_end_try();

	PHPUtils::SwapGlobalObject(previousGlobal, &EG(symbol_table) TSRMLS_CC);

    return Value::Null;
}

} // namespace kroll
