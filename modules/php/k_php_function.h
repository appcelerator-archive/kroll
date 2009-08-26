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
		KPHPFunction(const char *globalFunction);
		virtual ~KPHPFunction();
		SharedValue Call(const ValueList& args);
		virtual void Set(const char *name, SharedValue value);
		virtual SharedValue Get(const char *name);
		virtual SharedStringList GetPropertyNames();
		virtual SharedString DisplayString(int);
		virtual bool Equals(SharedKObject);
		bool PropertyExists(const char* property);

		private:
		zval zFunctionName;
		char* functionName;

	};
}
#endif
