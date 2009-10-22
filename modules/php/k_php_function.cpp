/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "php_module.h"

namespace kroll {

	KPHPFunction::KPHPFunction(const char* functionName) :
		KMethod("PHP.KPHPFunction"),
		methodName(functionName),
		zMethodName(0),
		globalObject(PHPUtils::GetCurrentGlobalObject())
	{
		MAKE_STD_ZVAL(zMethodName);
		ZVAL_STRINGL(zMethodName, methodName.c_str(), methodName.size(), 0);
		zval_addref_p(zMethodName);
	}

	KPHPFunction::~KPHPFunction()
	{
		if (zMethodName)
			zval_ptr_dtor(&zMethodName);
	}

	KValueRef KPHPFunction::Call(const ValueList& args)
	{
		TSRMLS_FETCH();

		HashTable* functionTable = 0;
		zend_class_entry* classEntry = 0;
		zval** passObject = 0;
		functionTable = EG(function_table);

		// Convert arguments
		zval** pzargs = (zval**) emalloc(sizeof(zval*) * args.size());
		for (int i = 0; i < args.size(); i++)
		{
			MAKE_STD_ZVAL(pzargs[i]);
			PHPUtils::ToPHPValue(args.at(i), &pzargs[i]);
		}

		int result;
		zval* zReturnValue;
		MAKE_STD_ZVAL(zReturnValue);

		KObjectRef previousGlobal(PHPUtils::GetCurrentGlobalObject());
		PHPUtils::SwapGlobalObject(this->globalObject, &EG(symbol_table) TSRMLS_CC);

		zend_try 
		{
			result = call_user_function(functionTable, passObject,
			zMethodName, zReturnValue, args.size(), (zval**) pzargs TSRMLS_CC);
		}
		zend_catch
		{
			result = FAILURE;
		}
		zend_end_try();

		PHPUtils::SwapGlobalObject(previousGlobal, &EG(symbol_table) TSRMLS_CC);

		// Cleanup the arguments.
		for (int i = 0; i < args.size(); i++)
			zval_ptr_dtor(&pzargs[i]);
		efree(pzargs);

		if (result == FAILURE)
		{
			if (classEntry)
			{
				throw ValueException::FromFormat("Couldn't execute method %s::%s", 
					classEntry->name, methodName.c_str());
			}
			else
			{
				throw ValueException::FromFormat("Couldn't execute function %s\n",
					methodName.c_str());
			}
			return Value::Undefined;
		}
		else if (zReturnValue)
		{
			KValueRef returnValue(PHPUtils::ToKrollValue(zReturnValue TSRMLS_CC));
			zval_ptr_dtor(&zReturnValue);
			return returnValue;
		}
		else
		{
			return Value::Undefined;
		}
	}

	void KPHPFunction::Set(const char *name, KValueRef value)
	{
		// TODO: PHP methods do not have properties. Should we should set
		// them on a StaticBoundObject here?
	}

	KValueRef KPHPFunction::Get(const char *name)
	{
		// TODO: PHP methods do not have properties. Should we should get
		// them from a StaticBoundObject here?
		return Value::Undefined;
	}

	bool KPHPFunction::Equals(KObjectRef other)
	{
		AutoPtr<KPHPFunction> phpOther = other.cast<KPHPFunction>();
		if (phpOther.isNull())
			return false;

		return methodName == phpOther->GetMethodName();
	}

	SharedString KPHPFunction::DisplayString(int levels)
	{
		std::string* displayString = new std::string("KPHPFunction: ");
		displayString->append(methodName);
		return displayString;
	}

	SharedStringList KPHPFunction::GetPropertyNames()
	{
		return new StringList();
	}
}
