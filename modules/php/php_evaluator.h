/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _PHP_EVALUATOR_H_
#define _PHP_EVALUATOR_H_

namespace kroll
{
	class PHPEvaluator : public StaticBoundObject
	{
		public:
		PHPEvaluator();
		void Evaluate(const ValueList& args, SharedValue result);
		void Preprocess(const ValueList& args, SharedValue result);
		
		protected:
		std::string CreateContextName();
		void FillServerVars(Poco::URI& uri, SharedKObject headers, std::string& httpMethod TSRMLS_DC);
		
	};
}

#endif

