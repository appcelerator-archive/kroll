/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_DEFINES_H_
#define _KR_DEFINES_H_

//
// a set of simple convenience defines around bindings and values
//

#define DECLAREBOUNDMETHOD(name) void name(const ValueList& args, SharedValue result);

#define SET_INT_PROP(name,value) this->Set(name,Value::NewInt(value));
#define SET_BOOL_PROP(name,value) this->Set(name,Value::NewBool(value));
#define SET_STRING_PROP(name,value) this->Set(name,Value::NewString(value));
#define SET_NULL_PROP(name) this->Set(name,Value::Null);
#define SET_UNDEFINED_PROP(name) this->Set(name,Value::Undefined);

#define GET_BOOL_FROM_STRING(value) (value=="1" || value == "true" || value == "on") ? true : false
#define GET_BOOL_PROP(from,into) into = from->IsBool() ? from->ToBool() : from->IsString() ? GET_BOOL_FROM_STRING(std::string(from->ToString())) : false;



#endif


