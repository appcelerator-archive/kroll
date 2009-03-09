/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef PYTHON_EVALUATOR_H_
#define PYTHON_EVALUATOR_H_

namespace kroll
{
	class PythonEvaluator : public BoundMethod
	{
		public:
		virtual SharedValue Call(const ValueList& args);
		virtual void Set(const char *, SharedValue);
		virtual SharedValue Get(const char *);
		virtual SharedStringList GetPropertyNames();

		private:
		static void ConvertLinefeeds(std::string &);
	};

	class DecoratorFunction : public BoundMethod
	{
		public:
		DecoratorFunction(SharedBoundObject);
		virtual SharedValue Call(const ValueList& args);
		virtual void Set(const char *, SharedValue);
		virtual SharedValue Get(const char *);
		virtual SharedStringList GetPropertyNames();

		private:
		SharedBoundObject window_global;
	};
}

#endif

