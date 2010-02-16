/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008-2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _JAVASCRIPT_METHODS_H_
#define _JAVASCRIPT_METHODS_H_

namespace kroll
{
namespace JavaScriptMethods
{

void Bind(KObjectRef global);
KValueRef SetTimeout(const ValueList& args);
KValueRef SetInterval(const ValueList& args);
KValueRef ClearTimeout(const ValueList& args);
KValueRef ClearInterval(const ValueList& args);

}
}
#endif
