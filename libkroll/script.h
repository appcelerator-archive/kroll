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
	class KROLL_API PreprocessData : public ReferenceCounted
	{
	public:
		BytesRef data;
		std::string mimeType;
	};
	
	class KROLL_API Script
	{
	public:
		static SharedPtr<Script> GetInstance();
		static void Initialize();
		static bool HasExtension(const char *url, const char *ext);
		static std::string GetExtension(const char *url);
		
		void AddScriptEvaluator(KObjectRef evaluator);
		void RemoveScriptEvaluator(KObjectRef evaluator);

		bool CanEvaluate(const char *mimeType);
		bool CanPreprocess(const char *url);
		KListRef GetEvaluators() { return evaluators; }

		KValueRef Evaluate(const char *mimeType, const char *name, const char *code, KObjectRef scope);
		AutoPtr<PreprocessData> Preprocess(const char *url, KObjectRef scope);
		
	protected:
		KListRef evaluators;
		static SharedPtr<Script> instance;
		
		Script() : evaluators(new StaticBoundList()) { }
		KObjectRef FindEvaluatorWithMethod(const char *method, const char *arg);
	};
}

#endif
