/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _RUBY_EVALUATOR_H_
#define _RUBY_EVALUATOR_H_

namespace kroll
{
	class RubyEvaluator : public KMethod
	{
		public:
		RubyEvaluator();
		~RubyEvaluator();

		virtual SharedValue Call(const ValueList& args);
		virtual void Set(const char *, SharedValue);
		virtual SharedValue Get(const char *);
		virtual SharedStringList GetPropertyNames();

		private:
		int next_id;
		std::string GetContextId(SharedKObject global);
		VALUE GetContext(SharedKObject global);
		void ContextToGlobal(VALUE ctx, SharedKObject o);
	};
}

#endif

