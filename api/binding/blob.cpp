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
	Blob::Blob() 
	{
		Create(NULL, 0);
	}

	Blob::Blob(char *buf)
	{
		// Assume this guy is null terminated. If not -- oops.
		Create(buf, strlen(buf));
	}

	Blob::Blob(char *buf, int len)
	{
		Create(buf, len);
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

	void Blob::Create(const char *buf, int length)
	{
		if (length > 0)
		{
			// Store the buffer with a null terminator so
			// that we can use it like a string later on.
			this->buffer = new char[length + 1];
			memcpy(this->buffer, buf, length);
			this->buffer[length] = '\0';
		}
		else
		{
			this->buffer = NULL;
		}

		this->length = length;
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
		this->Set("length", Value::NewInt(length));
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
		if (this->length == 0)
		{
			result->SetString("");
		}
		else
		{
			result->SetString(buffer);
		}
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
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/indexOf
		args.VerifyException("Blob.indexOf", "s,?i");

		if (this->length <= 0)
		{
			result->SetInt(-1);
		}
		else
		{
			std::string target = this->buffer;
			std::string needle = args.at(0)->ToString();
			int start = 0;
			if (args.size() > 1)
			{
				start = args.GetNumber(1);
				if (start < 0)
				{
					start = 0;
				}
			}
			result->SetInt(target.find(needle, start));
		}
	}

	void Blob::LastIndexOf(const ValueList& args, SharedValue result)
	{
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/lastIndexOf
		args.VerifyException("Blob.lastIndexOf", "s,?i");

		if (this->length <= 0)
		{
			result->SetInt(-1);
		}
		else
		{
			std::string target = this->buffer;
			std::string needle = args.at(0)->ToString();
			int start = target.size() + 1;
			if (args.size() > 1)
			{
				start = args.GetNumber(1);
				if (start < 0)
				{
					start = 0;
				}
			}
			result->SetInt(target.rfind(needle, start));
		}
	}

	void Blob::CharAt(const ValueList& args, SharedValue result)
	{
		// https://developer.mozilla.org/en/core_javascript_1.5_reference/global_objects/string/charat
		args.VerifyException("Blob.charAt", "n");
		int position = args.at(0)->ToInt();

		char buf[2] = {'\0', '\0'};
		if (position >= 0 && position < this->length)
		{
			buf[0] = this->buffer[position];
		}
		result->SetString(buf);
	}

	void Blob::Split(const ValueList& args, SharedValue result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/split
		// Except support for regular expressions
		args.VerifyException("Blob.split", "?s,i");

		SharedKList list = new StaticBoundList();
		result->SetList(list);

		std::string target = "";
		if (this->length > 0)
		{
			target = this->buffer;
		}
		else
		{
			list->Append(Value::NewString(target));
			return;
		}

		if (args.size() <= 0)
		{
			list->Append(Value::NewString(target));
			return;
		}

		std::string separator = args.GetString(0);
		int limit = -1;
		if (args.size() > 0)
		{
			limit = args.GetInt(1);
		}

		int current = 0;
		Poco::StringTokenizer tokens(target, separator);
		Poco::StringTokenizer::Iterator i = tokens.begin();
		while (i != tokens.end() && (limit < 0 || current < limit))
		{
			list->Append(Value::NewString((*i++)));
			current++;
		}
	}

	void Blob::Substring(const ValueList& args, SharedValue result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/substr
		args.VerifyException("Blob.substr", "i,?i");
		std::string target = "";
		if (this->length > 0)
		{
			target = this->buffer;
		}

		int start = args.GetInt(0);
		if (start > 0 && start >= this->length)
		{
			result->SetString("");
			return;
		}

		if (start < 0 && (-1*start) > this->length)
		{
			start = 0;
		}
		else if (start < 0)
		{
			start = this->length - start;
		}

		int length = this->length - start;
		if (args.size() > 1)
		{
			length = args.GetInt(1);
		}

		if (length <= 0)
		{
			result->SetString("");
			return;
		}

		std::string r = target.substr(start, length);
		result->SetString(r);
	}

	void Blob::ToLowerCase(const ValueList& args, SharedValue result)
	{
		if (this->length > 0)
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
		if (this->length > 0)
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
		args.VerifyException("Blob.replace", "s,s");

		if (this->length > 0)
		{
			std::string target = this->buffer;
			std::string find = args.GetString(0);
			std::string replace = args.GetString(1);
			std::string r = Poco::replace(target, find, replace);
			result->SetString(r);
		}
		else
		{
			result->SetNull();
		}
	}
}
