/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "php_module.h"

namespace kroll {

	KPHPFunction::KPHPFunction(zval* object, const char* methodName) :
		object(object),
		methodName(strdup(methodName)),
		zMethodName(0)
	{
		zval_addref_p(object);

		MAKE_STD_ZVAL(zMethodName);
		ZVAL_STRINGL(zMethodName, methodName, strlen(methodName), 0);
	}

	KPHPFunction::KPHPFunction(const char *functionName) :
		object(0),
		methodName(strdup(functionName)),
		zMethodName(0)
	{
		MAKE_STD_ZVAL(zMethodName);
		ZVAL_STRINGL(zMethodName, methodName, strlen(methodName), 0);
	}

	KPHPFunction::~KPHPFunction()
	{
		if (object)
			zval_delref_p(object);

		free(methodName);
		zval_ptr_dtor(&zMethodName);
	}

	SharedValue KPHPFunction::Call(const ValueList& args)
	{
		TSRMLS_FETCH();

		HashTable* functionTable = 0;
		zend_class_entry* classEntry = 0;
		zval** passObject = 0;
		if (object)
		{
			passObject = &object;
			classEntry = Z_OBJCE_P(object);
			functionTable = &classEntry->function_table;
		}
		else
		{
			functionTable = EG(function_table);
		}

        // Convert arguments
		zval** pzargs = (zval**) emalloc(sizeof(zval*) * args.size());
		for (int i = 0; i < args.size(); i++)
		{
			MAKE_STD_ZVAL(pzargs[i]);
			PHPUtils::ToPHPValue(args.at(i), &pzargs[i]);
		}

		zval* zReturnValue;
		MAKE_STD_ZVAL(zReturnValue);
		int result = call_user_function(functionTable, passObject,
			zMethodName, zReturnValue, args.size(), (zval**) pzargs TSRMLS_CC);

		// Cleanup the arguments.
		for (int i = 0; i < args.size(); i++)
			zval_ptr_dtor(&pzargs[i]);
		efree(pzargs);

		if (result == FAILURE)
		{
			if (classEntry)
			{
				throw ValueException::FromFormat("Couldn't execute method %s::%s", 
					classEntry->name, methodName);
			}
			else
			{
				throw ValueException::FromFormat("Couldn't execute function %s\n",
					methodName);
			}
			return Value::Undefined;
		}
		else if (zReturnValue)
		{
			SharedValue returnValue(PHPUtils::ToKrollValue(zReturnValue TSRMLS_CC));
			zval_ptr_dtor(&zReturnValue);
			return returnValue;
		}
		else
		{
			return Value::Undefined;
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
		AutoPtr<KPHPFunction> phpOther = other.cast<KPHPFunction>();
		if (phpOther.isNull())
			return false;
		
		TSRMLS_FETCH();
		return PHPUtils::PHPObjectsEqual(this->ToPHP(), phpOther->ToPHP() TSRMLS_CC);
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

	zval* KPHPFunction::ToPHP()
	{
		return this->object;
	}
}
