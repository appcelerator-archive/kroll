/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_EVALUATOR_H_
#define _PHP_EVALUATOR_H_

namespace kroll
{
	class PHPEvaluator : public KMethod
	{
		public:
		virtual SharedValue Call(const ValueList& args);
		virtual void Set(const char *, SharedValue);
		virtual SharedValue Get(const char *);
		virtual SharedStringList GetPropertyNames();
		
		protected:
		std::string CreateContextName();
		
	};
}

#endif

