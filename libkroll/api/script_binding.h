/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _SCRIPT_BINDING_H
#define _SCRIPT_BINDING_H

#include <kroll/kroll.h>

namespace kroll
{
	class KROLL_API ScriptBinding : public StaticBoundObject
	{
	public:
		ScriptBinding();
		
	protected:
		void _AddScriptEvaluator(const ValueList& args, KValueRef result);
		void _RemoveScriptEvaluator(const ValueList& args, KValueRef result);
		void _CanEvaluate(const ValueList& args, KValueRef result);
		void _CanPreprocess(const ValueList& args, KValueRef result);
		void _Evaluate(const ValueList& args, KValueRef result);
		void _Preprocess(const ValueList& args, KValueRef result);
	};
}

#endif
