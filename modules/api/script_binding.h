/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _SCRIPT_BINDING_H
#define _SCRIPT_BINDING_H

#include <kroll/kroll.h>
#include "api.h"

namespace kroll
{
	class KROLL_API_API ScriptBinding : public StaticBoundObject
	{
	public:
		ScriptBinding();
		
	protected:
		void _AddScriptEvaluator(const ValueList& args, SharedValue result);
		void _RemoveScriptEvaluator(const ValueList& args, SharedValue result);
		void _CanEvaluate(const ValueList& args, SharedValue result);
		void _CanPreprocess(const ValueList& args, SharedValue result);
		void _Evaluate(const ValueList& args, SharedValue result);
		void _Preprocess(const ValueList& args, SharedValue result);
	};
}

#endif
