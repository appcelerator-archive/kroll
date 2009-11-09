/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _RUBY_EVALUATOR_H_
#define _RUBY_EVALUATOR_H_

namespace kroll
{
	class RubyEvaluator : public StaticBoundObject
	{
		public:
		RubyEvaluator();
		~RubyEvaluator();

		void CanEvaluate(const ValueList& args, KValueRef result);
		void Evaluate(const ValueList& args, KValueRef result);

		private:
		std::string GetContextId(KObjectRef global);
		VALUE GetContext(KObjectRef global);
		void ContextToGlobal(VALUE ctx, KObjectRef o);
	};
}

#endif

