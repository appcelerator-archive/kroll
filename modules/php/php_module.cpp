/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "php_module.h"
#include <Poco/Path.h>

#ifdef ZTS
void ***tsrm_ls;
#endif

namespace kroll
{
	KROLL_MODULE(PHPModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	PHPModule* PHPModule::instance_ = NULL;
	bool PHPModule::buffering = false;
	std::ostringstream PHPModule::buffer;
	std::string PHPModule::mimeType("text/html");
	
	void PHPModule::Initialize()
	{
		PHPModule::instance_ = this;
		logger = Logger::Get("PHP");
		int argc = 1;
		char *argv[2] = { "php_kroll", NULL };

		php_embed_module.ub_write = PHPModule::UnbufferedWrite;
		php_embed_module.log_message = PHPModule::LogMessage;
		php_embed_module.ini_defaults = PHPModule::IniDefaults;
		php_embed_module.header_handler = PHPModule::HeaderHandler;

		php_embed_init(argc, argv PTSRMLS_CC);
		PHPUtils::InitializePHPKrollClasses();

		this->InitializeBinding();
		host->AddModuleProvider(this);
	}
	
	/*static*/
	void PHPModule::SetBuffering(bool buffering_)
	{
		if (buffering)
		{
			buffer.str("");
		}
		buffering = buffering_;
	}
	
	/*static*/
	int PHPModule::UnbufferedWrite(const char *str, unsigned int length TSRMLS_DC)
	{
		std::string string(str,length);
		
		// This shouldn't need to be thread safe right?
		if (buffering)
		{
			buffer << string;
		}
		else
		{
			PHPModule::Instance()->logger->Info(string.c_str());
		}
		return length;
	}
	
	// Forgive me Martin, borrowed from php_cli.c line 409
	#define INI_DEFAULT(name,value)\
		Z_SET_REFCOUNT(tmp, 0);\
		Z_UNSET_ISREF(tmp); \
		ZVAL_STRINGL(&tmp, zend_strndup(value, sizeof(value)-1), sizeof(value)-1, 0);\
		zend_hash_update(configuration, name, sizeof(name), &tmp, sizeof(zval), NULL);\
	
	/*static*/
	void PHPModule::IniDefaults(HashTable *configuration)
	{
		zval tmp;
		INI_DEFAULT("display_errors", "1");
	}
	
	/*static*/
	void PHPModule::LogMessage(char *message)
	{
		PHPModule::Instance()->logger->Debug(message);
	}

	/*static*/
	int PHPModule::HeaderHandler(sapi_header_struct *sapiHeader,
		sapi_header_op_enum op, sapi_headers_struct *sapi_headers TSRMLS_DC)
	{
		if (sapi_headers && sapi_headers->mimetype)
		{
			PHPModule::mimeType = sapi_headers->mimetype;
		}
		return op;
	}
	
	void PHPModule::Stop()
	{
		PHPModule::instance_ = NULL;

		SharedKObject global = this->host->GetGlobalObject();
		Script::GetInstance()->RemoveScriptEvaluator(this->binding);
		global->Set("PHP", Value::Undefined);
		this->binding->Set("evaluate", Value::Undefined);
		this->binding = NULL;
		PHPModule::instance_ = NULL;

		php_embed_shutdown(TSRMLS_C);
	}

	void PHPModule::InitializeBinding()
	{
		PHPModule::mimeType = SG(default_mimetype);
	
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new PHPEvaluator();
		global->Set("PHP", Value::NewObject(this->binding));
		Script::GetInstance()->AddScriptEvaluator(this->binding);
		
		zval *titaniumValue = PHPUtils::ToPHPValue(Value::NewObject(global));
		ZEND_SET_SYMBOL(&EG(symbol_table), "Titanium", titaniumValue);
	}

	const static std::string php_suffix = "module.php";

	bool PHPModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-php_suffix.length()) == php_suffix);
	}

	Module* PHPModule::CreateModule(std::string& path)
	{
		Logger *logger = Logger::Get("PHP");
		zend_first_try {
			std::string includeScript = "include '" + path + "';";
			if (SUCCESS != zend_eval_string((char *)includeScript.c_str(), NULL, (char *) path.c_str() TSRMLS_CC))
			{
				logger->Error("Error evaluating module at path: %s", path.c_str());
			}
		} zend_catch {
			logger->Error("Error evaluating module at path: %s", path.c_str());
		} zend_end_try();

		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-php_suffix.length()+4);
		std::string moduledir = path.substr(0,path.length()-basename.length()-4);

		logger->Info("Loading PHP path=%s", path.c_str());

		return new PHPModuleInstance(host, path, moduledir, name);
	}
}
