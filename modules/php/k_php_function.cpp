/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "php_module.h"

namespace kroll {

	KPHPFunction::KPHPFunction(const char* functionName) :
		functionName(strdup(functionName))
	{
		ZVAL_STRING(&zFunctionName, this->functionName, 1);
	}

	KPHPFunction::~KPHPFunction()
	{
		free(functionName);
		zval_dtor(&zFunctionName);
	}

	SharedValue KPHPFunction::Call(const ValueList& args)
	{
		TSRMLS_FETCH();

		zval* zargs = new zval[args.size()];
		for (int i = 0; i < args.size(); i++)
		{
			SharedValue value = args.at(i);
			zval *zargument = zargs + i;
			PHPUtils::ToPHPValue(args[i], &zargument);
		}

		zval* zReturnValue;
		ALLOC_INIT_ZVAL(zReturnValue);
		int result = call_user_function(EG(function_table), NULL,
			&zFunctionName, zReturnValue, args.size(), &zargs TSRMLS_CC);

		for (int i = 0; i < args.size(); i++)
		{
			zval_dtor(zargs + i);
		}
		delete [] zargs;

		if (result == FAILURE)
		{
			throw ValueException::FromFormat(
				"Failed to call PHP function: %s", functionName);
			return Value::Undefined;
		}
		else
		{
			SharedValue returnValue(PHPUtils::ToKrollValue(zReturnValue TSRMLS_CC));
			zval_dtor(zReturnValue);
			return returnValue;
		}
	}

	void KPHPFunction::Set(const char *name, SharedValue value)
	{
		// TODO: PHP methods do not have properties. Should we should set
		// them on a StaticBoundObject here?
	}

	SharedValue KPHPFunction::Get(const char *name)
	{
		// TODO: PHP methods do not have properties. Should we should get
		// them from a StaticBoundObject here?
		return Value::Undefined;
	}

	bool KPHPFunction::Equals(SharedKObject other)
	{
		return false;
	}

	SharedString KPHPFunction::DisplayString(int levels)
	{
		std::string* displayString = new std::string("KPHPFunction: ");
		displayString->append(functionName);
		return displayString;
	}

	SharedStringList KPHPFunction::GetPropertyNames()
	{
		return new StringList();
	}
}
