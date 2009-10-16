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

		void CanEvaluate(const ValueList& args, SharedValue result);
		void Evaluate(const ValueList& args, SharedValue result);

		private:
		std::string GetContextId(SharedKObject global);
		VALUE GetContext(SharedKObject global);
		void ContextToGlobal(VALUE ctx, SharedKObject o);
	};
}

#endif

