/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef PYTHON_EVALUATOR_H_
#define PYTHON_EVALUATOR_H_

#include <kroll/Interpreter.h>

namespace kroll {

class PythonInterpreter : public Interpreter {
public:
	PythonInterpreter();

    virtual KValueRef EvaluateFile(const char* filepath, KObjectRef context);
};

} // namespace kroll

#endif