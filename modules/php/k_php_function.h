/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _K_PHP_FUNCTION_H_
#define _K_PHP_FUNCTION_H_

namespace kroll
{
	class KPHPFunction : public KMethod
	{
		public:
		KPHPFunction(const char *functionName);

		virtual ~KPHPFunction();
		KValueRef Call(const ValueList& args);
		virtual void Set(const char *name, KValueRef value);
		virtual KValueRef Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		virtual SharedString DisplayString(int);
		virtual bool Equals(KObjectRef);
		bool PropertyExists(const char* property);

		inline std::string& GetMethodName() { return methodName; }

		private:
		std::string methodName;
		zval* zMethodName;
		KObjectRef globalObject;
	};
}
#endif
