/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "blob.h"
#include <cstring>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>

namespace kroll
{

	Blob::Blob(char *buf, int len)
	{
		Create(static_cast<const char*>(buf), len);
	}

	Blob::Blob(const char *buffer, int len)
	{
		Create(buffer, len);
	}

	Blob::Blob(std::string str)
	{
		Create(str.c_str(), str.length());
	}

	Blob::Blob(std::string& str)
	{
		Create(str.c_str(), str.length());
	}

	void Blob::Create(const char *buf, int len)
	{
		if (len > 0)
		{
			this->buffer = new char[len + 1];
			memcpy(this->buffer, buf, len);
			this->buffer[len]='\0'; // null terminate buffer
		}
		else
		{
			this->buffer = NULL;
		}

		this->length = len;
		this->SetMethod("toString", &Blob::ToString);
		this->SetMethod("get", &Blob::Get);

		// mimic some string operations to make it more 
		// friendly when using a Blob in Javascript
		this->SetMethod("indexOf", &Blob::IndexOf);
		this->SetMethod("lastIndexOf", &Blob::LastIndexOf);
		this->SetMethod("charAt", &Blob::CharAt);
		this->SetMethod("split", &Blob::Split);
		this->SetMethod("substring", &Blob::Substring);
		this->SetMethod("substr", &Blob::Substring);
		this->SetMethod("toLowerCase", &Blob::ToLowerCase);
		this->SetMethod("toUpperCase", &Blob::ToUpperCase);
		this->SetMethod("replace", &Blob::Replace);
		this->Set("length", Value::NewInt(len));
	}

	Blob::~Blob()
	{
		if (this->buffer!=NULL)
		{
			delete [] this->buffer;
		}
		this->buffer = NULL;
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

	void Blob::IndexOf(const ValueList& args, SharedValue result)
	{
		if (this->length > 0)
		{
			std::string subject = args.at(0)->ToString();
			std::string target = this->buffer;
			result->SetInt(target.find(subject));
		}
		else
		{
			result->SetInt(-1);
		}
	}

	void Blob::LastIndexOf(const ValueList& args, SharedValue result)
	{
		if (this->length > 0)
		{
			std::string subject = args.at(0)->ToString();
			std::string target = this->buffer;
			result->SetInt(target.find_last_of(subject));
		}
		else
		{
			result->SetInt(-1);
		}
	}

	void Blob::CharAt(const ValueList& args, SharedValue result)
	{
		args.VerifyException("charAt", "n");
		int position = args.at(0)->ToInt();

		if (position < 0)
		{
			result->SetBool(true);
			return;
		}

		char buf[2] = {'\0', '\0'};
		if (position >= 0 && position < this->length)
		{
			buf[0] = this->buffer[position];
		}
		// Else return an empty string 
		// https://developer.mozilla.org/en/core_javascript_1.5_reference/global_objects/string/charat
		result->SetString(buf);
	}

	void Blob::Split(const ValueList& args, SharedValue result)
	{
		SharedKList list = new StaticBoundList();
		if (this->length > 0)
		{
			std::string str(this->buffer);
			std::string sep = args.at(0)->ToString();
			Poco::StringTokenizer tok(str,sep);
			Poco::StringTokenizer::Iterator i = tok.begin();
			while(i!=tok.end())
			{
				std::string token = (*i++);
				list->Append(Value::NewString(token));
			}
		}
		result->SetList(list);
	}

	void Blob::Substring(const ValueList& args, SharedValue result)
	{
		int a = args.at(0)->ToInt();
		if (this->length > 0 && a < this->length)
		{
			std::string target = this->buffer;
			if (args.size()==1)
			{
				std::string r = target.substr(a);
				result->SetString(r);
				return;
			}
			else
			{
				int b = args.at(1)->ToInt();
				if (b < this->length)
				{
					std::string r = target.substr(a,b);
					result->SetString(r);
					return;
				}
			}
		}
		result->SetNull();
	}

	void Blob::ToLowerCase(const ValueList& args, SharedValue result)
	{
		if (this->length>0)
		{
			std::string target = this->buffer;
			std::string r = Poco::toLower(target);
			result->SetString(r);
		}
		else
		{
			result->SetNull();
		}
	}

	void Blob::ToUpperCase(const ValueList& args, SharedValue result)
	{
		if (this->length>0)
		{
			std::string target = this->buffer;
			std::string r = Poco::toUpper(target);
			result->SetString(r);
		}
		else
		{
			result->SetNull();
		}
	}

	void Blob::Replace(const ValueList& args, SharedValue result)
	{
		if (args.size()!=2)
		{
			throw ValueException::FromString("invalid arguments");
		}
		if (this->length>0)
		{
			std::string target = this->buffer;
			std::string find = args.at(0)->ToString();
			std::string replace = args.at(1)->ToString();
			std::string r = Poco::replace(target,find,replace);
			result->SetString(r);
		}
		else
		{
			result->SetNull();
		}
	}
}
