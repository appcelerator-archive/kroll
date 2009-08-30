/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_SCRIPT_H_
#define _KR_SCRIPT_H_

#include <kroll/kroll.h>

namespace kroll
{
	class KROLL_API Script
	{
	public:
		static SharedPtr<Script> GetInstance();
		static void Initialize();
		static bool HasExtension(const char *url, const char *ext);
		
		void AddScriptEvaluator(SharedKObject evaluator);
		void RemoveScriptEvaluator(SharedKObject evaluator);

		bool CanEvaluate(const char *mimeType);
		bool CanPreprocess(const char *url);
		SharedKList GetEvaluators() { return evaluators; }

		SharedValue Evaluate(const char *mimeType, const char *name, const char *code, SharedKObject scope);
		SharedString Preprocess(const char *url, SharedKObject scope);
		
	protected:
		SharedKList evaluators;
		static SharedPtr<Script> instance;
		
		Script() : evaluators(new StaticBoundList()) { }
		SharedKObject FindEvaluatorWithMethod(const char *method, const char *arg);
	};
}

#endif
