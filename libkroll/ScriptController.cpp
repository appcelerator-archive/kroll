/*
 * Copyright (c) 2011 Appcelerator, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ScriptController.h"

#include <string.h>

#include "Interpreter.h"

namespace kroll {

ScriptController::ScriptController()
{
}

void ScriptController::AddInterpreter(Interpreter* interpreter, const char* supportedScriptTypes[])
{
    const char* type;
    int i = 0;

    while ((type = supportedScriptTypes[i++])) {
        interpreters[type] = interpreter;
    }
}

void ScriptController::RemoveInterpreter(Interpreter* interpreter)
{
    InterpreterMapping::iterator i = interpreters.begin();
    while (i != interpreters.end()) {
        if (i->second == interpreter)
            interpreters.erase(i++);
        else
            ++i;
    }
}

KValueRef ScriptController::EvaluateFile(const char* filepath, KObjectRef context)
{
    const char* scriptType = strrchr(filepath, '.');
    if (!scriptType)
        throw ValueException::FromFormat("Invalid script path, missing extension: %s", filepath);
    ++scriptType;

    Interpreter* interpreter = findInterpreterForType(scriptType);
    return interpreter->EvaluateFile(filepath, context);
}

Interpreter* ScriptController::findInterpreterForType(const char* scriptType)
{
    InterpreterMapping::iterator i = interpreters.find(scriptType);
    if (i == interpreters.end())
        throw ValueException::FromFormat("Cannot evalute file, invalid script type: %s", scriptType);

    return i->second;
}

} // namespace kroll
