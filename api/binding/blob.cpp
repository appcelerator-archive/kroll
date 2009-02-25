/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "blob.h"

namespace kroll
{

	Blob::Blob(char *buf, int len) : length(len)
	{
		this->buffer = new char[len];
		memcpy(this->buffer,buf,len);
		this->SetMethod("toString",&Blob::ToString);
		this->SetMethod("get",&Blob::Get);
		this->Set("length",Value::NewInt(len));
	}
	Blob::~Blob()
	{
		delete [] this->buffer;
		this->buffer = 0;
		this->length = 0;
	}
	void Blob::ToString(const ValueList& args, SharedValue result)
	{
		result->SetString(buffer);
	}
	void Blob::Get(const ValueList& args, SharedValue result)
	{
		result->SetVoidPtr(buffer);
	}
	void Blob::Length(const ValueList& args, SharedValue result)
	{
		result->SetInt(length);
	}
}
