/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "php_module.h"
#include <sstream>
#include <map>
#include <algorithm>
 
using std::string;
using std::map;

namespace kroll
{
	PHPEvaluator::PHPEvaluator()
		: StaticBoundObject("PHP.PHPEvaluator")
	{
		/**
		 * @notiapi(method=True,name=PHP.canEvaluate,since=0.7)
		 * @notiarg[String, mimeType] Code mime type
		 * @notiresult[bool] whether or not the mimetype is understood by PHP
		 */
		SetMethod("canEvaluate", &PHPEvaluator::CanEvaluate);

		/**
		 * @notiapi(method=True,name=PHP.evaluate,since=0.7) Evaluates a string as PHP code
		 * @notiarg[String, mimeType] Code mime type (normally "text/php")
		 * @notiarg[String, name] name of the script source
		 * @notiarg[String, code] PHP script code
		 * @notiarg[Object, scope] global variable scope
		 * @notiresult[Any] result of the evaluation
		 */
		SetMethod("evaluate", &PHPEvaluator::Evaluate);

		/**
		 * @notiapi(method=True,name=PHP.canPreprocess,since=0.7)
		 * @notiarg[String, url] URL to preprocess
		 * @notiresult[bool] whether or not the mimetype is understood by PHP
		 */
		SetMethod("canPreprocess", &PHPEvaluator::CanPreprocess);

		/**
		 * @notiapi(method=True,name=PHP.preprocess,since=0.7)
		 * @notiapiRuns a string and URL through preprocessing
		 * @notiarg[String, url] URL used to load this resource
		 * @notiarg[Object, scope] Global variables to bind for PHP
		 * @notiresult[String] result of the evaluation
		 */
		SetMethod("preprocess", &PHPEvaluator::Preprocess);
	}

	void PHPEvaluator::CanEvaluate(const ValueList& args, KValueRef result)
	{
		args.VerifyException("canEvaluate", "s");
		
		result->SetBool(false);
		string mimeType(args.GetString(0));
		if (mimeType == "text/php")
		{
			result->SetBool(true);
		}
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

	void PHPEvaluator::Evaluate(const ValueList& args, KValueRef result)
	{
		static Poco::Mutex evaluatorMutex;
		Poco::Mutex::ScopedLock evaluatorLock(evaluatorMutex);

		args.VerifyException("evaluate", "s s s o");

		TSRMLS_FETCH();
		string mimeType(args.GetString(0));
		string name(args.GetString(1));
		string code(args.GetString(2));
		KObjectRef windowGlobal(args.GetObject(3));
		KValueRef kv(Value::Undefined);

		// Contexts must be the same for runs with the same global object.
		string contextId(GetContextId(windowGlobal));
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
		PHPUtils::SwapGlobalObject(windowGlobal, &EG(symbol_table) TSRMLS_CC);

		zend_first_try
		{
			zend_eval_string((char *) codeString.str().c_str(), NULL, 
				(char *) name.c_str() TSRMLS_CC);
		}
		zend_catch
		{
			Logger::Get("PHP")->Error("Evaluation of script failed");
		}
		zend_end_try();

		PHPUtils::SwapGlobalObject(previousGlobal, &EG(symbol_table) TSRMLS_CC);
		result->SetValue(kv);
	}

	void PHPEvaluator::CanPreprocess(const ValueList& args, KValueRef result)
	{
		args.VerifyException("canPreprocess", "s");

		string url(args.GetString(0));
		Poco::URI uri(url);

		result->SetBool(false);
		if (Script::HasExtension(uri.getPath().c_str(), "php"))
		{
			result->SetBool(true);
		}
	}

	void PHPEvaluator::FillGet(Poco::URI& uri TSRMLS_DC)
	{
		Poco::StringTokenizer tokens(uri.getQuery(), "&=");
		Poco::StringTokenizer::Iterator iter = tokens.begin();
		
		for (; iter != tokens.end(); iter++)
		{
			string key(*iter);
			string value(*(++iter));

			zval *val;
			ALLOC_INIT_ZVAL(val);
			ZVAL_STRING(val, (char *) value.c_str(), 1);
			zend_hash_add(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]),
				(char *) key.c_str(), key.size()+1, &val, sizeof(zval*), NULL);
		}
	}
	
	void PHPEvaluator::Preprocess(const ValueList& args, KValueRef result)
	{
		args.VerifyException("preprocess", "s o");

		string url(args.GetString(0));
		Logger::Get("PHP")->Debug("preprocessing php => %s", url.c_str());

		Poco::URI uri(url);
		string path(URLUtils::URLToPath(url));

		KObjectRef scope = args.GetObject(1);
		TSRMLS_FETCH();

		PHPModule::SetBuffering(true);
		PHPModule::Instance()->PushURI(uri);
		
		// These variables are normally initialized by php_module_startup
		// but we do not call that function, so we manually initialize.
		PG(header_is_being_sent) = 0;
		SG(request_info).headers_only = 0;
		SG(request_info).argv0 = NULL;
		SG(request_info).argc= 0;
		SG(request_info).argv= (char **) NULL;
		php_request_startup(TSRMLS_C);
		FillGet(uri TSRMLS_CC);

		// This seems to be needed to make PHP actually give  us errors
		// at parse/compile time -- see: main/main.c line 969
		PG(during_request_startup) = 0;

		// Convert the path to the system codepage.
		path = UTF8ToSystem(path);

		zend_file_handle script;
		script.type = ZEND_HANDLE_FP;
		script.filename = (char*) path.c_str();
		script.opened_path = NULL;
		script.free_filename = 0;
		script.handle.fp = fopen(script.filename, "rb");

		zend_first_try
		{
			php_execute_script(&script TSRMLS_CC);
		}
		zend_catch
		{

		}
		zend_end_try();

		string output(PHPModule::GetBuffer().str());
		KObjectRef o = new StaticBoundObject();
		o->SetObject("data", new Bytes(output.c_str(), output.size(), true));
		o->SetString("mimeType", PHPModule::GetMimeType().c_str());
		result->SetObject(o);

		PHPModule::Instance()->PopURI();
		PHPModule::SetBuffering(false);
	}
}
