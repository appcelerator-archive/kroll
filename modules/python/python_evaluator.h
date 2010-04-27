/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef PYTHON_EVALUATOR_H_
#define PYTHON_EVALUATOR_H_

#include <Poco/StringTokenizer.h>

namespace kroll
{
	class PythonEvaluator : public StaticBoundObject
	{
	public:
		PythonEvaluator();
		void Evaluate(const ValueList& args, KValueRef result);
		void CanEvaluate(const ValueList& args, KValueRef result);

	private:
		static void UnindentCode(std::string &code);
		static void DictToKObjectProps(PyObject* map, KObjectRef o);
		static void KObjectPropsToDict(KObjectRef o, PyObject* pyobj);
	};
}

#endif

