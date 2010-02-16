/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "php_module.h"
#include <Poco/Path.h>

extern "C"
{
	int php_load_extension(char *filename, int type, int start_now TSRMLS_DC);

	EXPORT PHPModule* CreateModule(Host *host, const char* path)
	{
		return new PHPModule(host, path);
	}
}

#ifdef ZTS
void ***tsrm_ls;
#endif

namespace kroll
{
	static Logger* logger = Logger::Get("PHPModule");
	const static std::string phpSuffix("module.php");
	static bool buffering = false;

	PHPModule* PHPModule::instance_ = NULL;
	std::ostringstream PHPModule::buffer;
	std::string PHPModule::mimeType("text/html");

	static int UnbufferedWrite(const char *, unsigned int TSRMLS_DC);
	static void SetIniDefault(HashTable*, const char*, const char*);
	static void IniDefaults(HashTable*);
	static void LogMessage(char*);
	static int HeaderHandler(sapi_header_struct*, sapi_header_op_enum, 
		sapi_headers_struct* TSRMLS_DC);
	static void RegisterServerVariables(zval *tracks_var_array TSRMLS_DC);

	void PHPModule::Initialize()
	{
		PHPModule::instance_ = this;
		int argc = 1;
		char *argv[2] = { "php_kroll", NULL };

		php_embed_module.ub_write = UnbufferedWrite;
		php_embed_module.log_message = LogMessage;
		php_embed_module.ini_defaults = IniDefaults;
		php_embed_module.header_handler = HeaderHandler;
		php_embed_module.register_server_variables = RegisterServerVariables;
		php_embed_module.phpinfo_as_text = 1;
		php_embed_init(argc, argv PTSRMLS_CC);

		PHPUtils::InitializePHPKrollClasses();
		this->InitializeBinding();
		host->AddModuleProvider(this);

		std::string resourcesPath(host->GetApplication()->GetResourcesPath());
		zend_alter_ini_entry("include_path", sizeof("include_path"),
			(char*) resourcesPath.c_str(), resourcesPath.size(),
			ZEND_INI_USER, ZEND_INI_STAGE_RUNTIME);

#ifdef OS_WIN32
		// Manually load some PHP extensions for Windows.
		TSRMLS_FETCH();
		std::string phpPath(UTF8ToSystem(this->GetPath()));

		std::string modPath(FileUtils::Join(phpPath.c_str(), "php_gd2.dll", 0));
		php_load_extension((char*) modPath.c_str(), 1, 1 TSRMLS_CC);
		modPath = FileUtils::Join(phpPath.c_str(), "php_openssl.dll", 0);
		php_load_extension((char*) modPath.c_str(), 1, 1 TSRMLS_CC);
		modPath = FileUtils::Join(phpPath.c_str(), "php_curl.dll", 0);
		php_load_extension((char*) modPath.c_str(), 1, 1 TSRMLS_CC);
		modPath = FileUtils::Join(phpPath.c_str(), "php_xls.dll", 0);
		php_load_extension((char*) modPath.c_str(), 1, 1 TSRMLS_CC);
#endif
	}

	/*static*/
	void PHPModule::SetBuffering(bool newBuffering)
	{
		if (buffering)
		{
			buffer.str("");
		}
		buffering = newBuffering;
	}

	void PHPModule::Stop()
	{
		PHPModule::instance_ = NULL;

		KObjectRef global = this->host->GetGlobalObject();
		Script::GetInstance()->RemoveScriptEvaluator(this->binding);
		global->Set("PHP", Value::Undefined);
		this->binding->Set("evaluate", Value::Undefined);

		this->binding = 0;
		PHPModule::instance_ = 0;

		php_embed_shutdown(TSRMLS_C);
	}

	void PHPModule::InitializeBinding()
	{
		PHPModule::mimeType = SG(default_mimetype);

		KObjectRef global = this->host->GetGlobalObject();
		this->binding = new PHPEvaluator();
		global->Set("PHP", Value::NewObject(this->binding));
		Script::GetInstance()->AddScriptEvaluator(this->binding);

		zval *titaniumValue = PHPUtils::ToPHPValue(Value::NewObject(global));
		ZEND_SET_SYMBOL(&EG(symbol_table), PRODUCT_NAME, titaniumValue);
	}


	bool PHPModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-phpSuffix.length()) == phpSuffix);
	}

	Module* PHPModule::CreateModule(std::string& path)
	{
		zend_first_try
		{
			std::string includeScript = "include '" + path + "';";
			if (SUCCESS != zend_eval_string((char *) includeScript.c_str(),
				NULL, (char *) path.c_str() TSRMLS_CC))
				logger->Error("Error evaluating module at path: %s", path.c_str());
		}
		zend_catch
		{
			logger->Error("Error evaluating module at path: %s", path.c_str());
		}
		zend_end_try();

		Poco::Path p(path);
		std::string name(p.getBaseName());
		std::string moduledir(p.makeParent().toString());
		logger->Info("Loading PHP module name=%s path=%s", name.c_str(), path.c_str());

		return new PHPModuleInstance(host, path, moduledir, name);
	}

	static int UnbufferedWrite(const char *str, unsigned int length TSRMLS_DC)
	{
		std::string string(str, length);
		std::ostringstream& buffer = PHPModule::GetBuffer();

		// This shouldn't need to be thread safe right?
		if (buffering)
		{
			buffer << string;
		}
		else
		{
			// Other language modules ship their output straight to stdout
			// so we might as well do the same here, rather than sending
			// it through the Logger. 
			std::cout << string;
		}
		return length;
	}

	static void SetIniDefault(HashTable* config, const char* name, const char* value)
	{
		// Forgive me Martin, borrowed from php_cli.c line 409
		// Thou are forgiven.
		zval tmp;
		Z_SET_REFCOUNT(tmp, 0);
		Z_UNSET_ISREF(tmp);
		ZVAL_STRINGL(&tmp, zend_strndup(value, sizeof(value)-1), sizeof(value)-1, 0);
		zend_hash_update(config, name, sizeof(name), &tmp, sizeof(zval), NULL);
	}

	static void IniDefaults(HashTable* configuration)
	{
		SetIniDefault(configuration, "display_errors", "1");
	}

	static void LogMessage(char* message)
	{
		logger->Debug(message);
	}

	static int HeaderHandler(sapi_header_struct* sapiHeader,
		sapi_header_op_enum op, sapi_headers_struct* sapiHeaders TSRMLS_DC)
	{
		if (sapiHeaders && sapiHeaders->mimetype)
		{
			std::string& mimeType = PHPModule::GetMimeType();
			mimeType = sapiHeaders->mimetype;
		}
		return op;
	}
	
	static void RegisterServerVariables(zval *tracks_var_array TSRMLS_DC)
	{
		Poco::URI* uri = PHPModule::Instance()->GetURI();
		if (uri)
		{
			php_register_variable("SCRIPT_NAME", (char*)uri->getPath().c_str(), tracks_var_array TSRMLS_CC);
			php_register_variable("REQUEST_URI", (char*)uri->getPathEtc().c_str(), tracks_var_array TSRMLS_CC);
		}
	}
}
