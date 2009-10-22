/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _K_PHP_METHOD_H_
#define _K_PHP_METHOD_H_

namespace kroll
{
	class KPHPMethod : public KMethod
	{
		public:
		KPHPMethod(zval* object, const char* methodName);
		KPHPMethod(const char *functionName);

		virtual ~KPHPMethod();
		KValueRef Call(const ValueList& args);
		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		virtual SharedString DisplayString(int);
		virtual bool Equals(KObjectRef);
		bool PropertyExists(const char* property);
		zval* ToPHP();

		private:
		zval* object;
		char* methodName;
		zval* zMethodName;
		KObjectRef globalObject;
	};
}
#endif
