/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "bytes.h"
#include <cstring>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Data/BLOB.h>

namespace kroll
{
	Bytes::Bytes() :
		StaticBoundObject("Bytes"),
		buffer(0),
		size(0)
	{
		this->SetupBinding();
	}

	Bytes::Bytes(size_t size) :
		StaticBoundObject("Bytes"),
		size(size)
	{
		this->buffer = new char[size];
		this->SetupBinding();
	}

	Bytes::Bytes(BytesRef source, size_t offset, size_t length) :
		StaticBoundObject("Bytes")
	{
		this->size = (length > 0) ? length : source->Length() - offset;
		this->buffer = source->Pointer() + offset;
		this->source = source;
		this->SetupBinding();
	}

	Bytes::Bytes(std::string& str) : StaticBoundObject("Bytes")
	{
		this->size = str.length();
		this->buffer = new char[this->size];
		memcpy(this->buffer, str.c_str(), this->size);
		this->SetupBinding();
	}

	Bytes::Bytes(const char* str, size_t length) :
		StaticBoundObject("Bytes")
	{
		this->size = (length > 0) ? length : strlen(str);
		this->buffer = new char[this->size];
		memcpy(this->buffer, str, this->size);
		this->SetupBinding();
	}

	Bytes::~Bytes()
	{
		if (this->source.isNull() && this->buffer)
			delete [] this->buffer;
	}

	size_t Bytes::ExtraMemoryCost()
	{
		return this->size;
	}

	size_t Bytes::Write(const char* data, size_t length, size_t offset)
	{
		size_t maxWriteSize = this->size - offset;
		size_t writeSize = (length > maxWriteSize) ? maxWriteSize : length;
		memcpy(this->buffer + offset, data, writeSize);
		return writeSize;
	}

	size_t Bytes::Write(BytesRef source, size_t offset)
	{
		return this->Write(source->Pointer(), source->Length(), offset);
	}

	std::string Bytes::AsString()
	{
		if (this->size > 0)
			return std::string(this->buffer, this->size);
		else
			return std::string("");
	}

	void Bytes::SetupBinding()
	{
		this->SetMethod("write", &Bytes::_Write);
		this->SetMethod("toString", &Bytes::_ToString);
		this->SetMethod("indexOf", &Bytes::_IndexOf);
		this->SetMethod("lastIndexOf", &Bytes::_LastIndexOf);
		this->SetMethod("charAt", &Bytes::_CharAt);
		this->SetMethod("byteAt", &Bytes::_ByteAt);
		this->SetMethod("split", &Bytes::_Split);
		this->SetMethod("substring", &Bytes::_Substring);
		this->SetMethod("substr", &Bytes::_Substr);
		this->SetMethod("toLowerCase", &Bytes::_ToLowerCase);
		this->SetMethod("toUpperCase", &Bytes::_ToUpperCase);
		this->SetMethod("concat", &Bytes::_Concat);
		this->SetMethod("slice", &Bytes::_Slice);

		this->Set("length", Value::NewInt(this->size));
	}

	void Bytes::_Write(const ValueList& args, KValueRef result)
	{
		args.VerifyException("write", "s|o ?n");
		int offset = args.GetInt(1, 0);
		int bytesWritten;

		if (args.at(0)->IsString())
		{
			const char* str = args.at(0)->ToString();
			size_t length = strlen(str);
			bytesWritten = Write(str, length, offset);
		}
		else
		{
			BytesRef data(args.GetObject(0).cast<Bytes>());
			if (data.isNull())
			{
				throw ValueException::FromString("May only write strings or Bytes object");
			}
			bytesWritten = Write(data, offset);
		}

		result->SetInt(bytesWritten);
	}

	void Bytes::_ToString(const ValueList& args, KValueRef result)
	{
		std::string str(this->AsString());
		result->SetString(str);
	}

	void Bytes::_IndexOf(const ValueList& args, KValueRef result)
	{
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/indexOf
		args.VerifyException("Bytes.indexOf", "s,?i");

		std::string target(this->AsString());
		int start = args.GetInt(1, 0);
		if (start < 0) start = 0;
	 	size_t pos = target.find(args.GetString(0), start);

		if (pos == std::string::npos)
		{
			// No matches found
			result->SetInt(-1);
		}
		else
		{
			result->SetInt(pos);
		}
	}

	void Bytes::_LastIndexOf(const ValueList& args, KValueRef result)
	{
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/lastIndexOf
		args.VerifyException("Bytes.lastIndexOf", "s,?i");

		std::string target(this->AsString());
		int start = args.GetInt(1, target.length() + 1);
		if (start < 0) start = 0;
		size_t pos = target.rfind(args.GetString(0), start);

		if (pos == std::string::npos)
		{
			// No matches found
			result->SetInt(-1);
		}
		else
		{
			result->SetInt(pos);
		}
	}

	void Bytes::_CharAt(const ValueList& args, KValueRef result)
	{
		// https://developer.mozilla.org/en/core_javascript_1.5_reference/global_objects/string/charat
		args.VerifyException("Bytes.charAt", "n");
		size_t position = args.GetInt(0);

		char buf[2] = {'\0', '\0'};
		if (position >= 0 && position < this->size)
		{
			buf[0] = this->buffer[position];
		}
		result->SetString(buf);
	}
	
	void Bytes::_ByteAt(const ValueList& args, KValueRef result)
	{
		args.VerifyException("Bytes.byteAt", "n");
		size_t position = args.GetInt(0);
		
		if (position >= 0 && position < this->size)
		{
			result->SetInt(static_cast<unsigned char>(this->buffer[position]));
		}
	}

	void Bytes::_Split(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/split
		// Except support for regular expressions
		args.VerifyException("Bytes.split", "?s,i");

		KListRef list = new StaticBoundList();
		result->SetList(list);

		std::string target = "";
		if (this->size > 0)
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

		int limit = args.GetInt(1, INT_MAX);

		// We could use Poco's tokenizer here, but it doesn't split strings
		// like "abc,def,," -> ['abc', 'def', '', ''] correctly. It produces
		// ['abc', 'def', ''] which is a different behavior than the JS split.
		// So we roll our own for now -- it's not very efficient right now, but
		// it should be correct.
		size_t next = target.find(separator);
		while (target.size() > 0 && next != std::string::npos)
		{
			std::string token;
			if (separator.size() == 0)
			{
				token = target.substr(0, 1);
			}
			else
			{
				token = target.substr(0, next);
			}
			target = target.substr(next + 1);
			next = target.find(separator);

			if ((int) list->Size() >= limit)
				return;

			list->Append(Value::NewString(token));
		}

		if ((int) list->Size() < limit && separator.size() != 0)
			list->Append(Value::NewString(target));
	}

	void Bytes::_Substr(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/substr
		args.VerifyException("Bytes.substr", "i,?i");
		std::string target(this->buffer, this->size);

		int start = args.GetInt(0);
		if (start > 0 && start >= (int)target.length())
		{
			result->SetString("");
			return;
		}

		if (start < 0 && (-1*start) > (int)target.length())
		{
			start = 0;
		}
		else if (start < 0)
		{
			start = target.length() + start;
		}

		long length = target.length() - start;
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

	void Bytes::_Substring(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/substring
		args.VerifyException("Bytes.substring", "i,?i");
		std::string target = "";
		if (this->size > 0)
		{
			target = this->buffer;
		}

		long indexA = args.GetInt(0);
		if (indexA < 0)
			indexA = 0;
		if (indexA > (long) target.size())
			indexA = target.size();

		if (args.size() < 2)
		{
			std::string r = target.substr(indexA);
			result->SetString(r);
		}
		else
		{
			long indexB = args.GetInt(1);
			if (indexB < 0)
				indexB = 0;
			if (indexB > (long) target.size())
				indexB = target.size();

			if (indexA == indexB)
			{
				result->SetString("");
				return;
			}
			if (indexA > indexB)
			{
				long temp = indexA;
				indexA = indexB;
				indexB = temp;
			}
			std::string r = target.substr(indexA, indexB - indexA);
			result->SetString(r);
		}
	}

	void Bytes::_ToLowerCase(const ValueList& args, KValueRef result)
	{
		if (this->size > 0)
		{
			std::string target(this->buffer, this->size);
			std::string r = Poco::toLower(target);
			result->SetString(r);
		}
		else
		{
			result->SetNull();
		}
	}

	void Bytes::_ToUpperCase(const ValueList& args, KValueRef result)
	{
		if (this->size > 0)
		{
			std::string target(this->buffer, this->size);
			std::string r = Poco::toUpper(target);
			result->SetString(r);
		}
		else
		{
			result->SetString("");
		}
	}
	
	BytesRef Bytes::Concat(std::vector<BytesRef>& bytes)
	{
		size_t size = 0;
		for (size_t i = 0; i < bytes.size(); i++)
		{
			size += bytes.at(i)->Length();
		}

		BytesRef newBytes = new Bytes(size);

		// Write the rest of the Bytes objects
		size_t bytesConcated = 0;
		std::vector<BytesRef>::iterator i = bytes.begin();
		while (i != bytes.end())
		{
			BytesRef source = *i++;
			newBytes->Write(source, bytesConcated);
			bytesConcated += source->Length();
		}

		return newBytes;
	}

	void Bytes::_Concat(const ValueList& args, KValueRef result)
	{
		std::vector<BytesRef> bytes;
		bytes.push_back(BytesRef(this, true));

		for (size_t i = 0; i < args.size(); i++)
		{
			if (args.at(i)->IsObject())
			{
				BytesRef bytesObject(args.GetObject(i).cast<Bytes>());
				if (!bytesObject.isNull());
				{
					bytes.push_back(bytesObject);
				}
			}
			else if (args.at(i)->IsString())
			{
				std::string str(args.GetString(i));
				bytes.push_back(new Bytes(str));
			}
		}

		BytesRef newBytes = Bytes::Concat(bytes);
		result->SetObject(newBytes);
	}

	void Bytes::_Slice(const ValueList& args, KValueRef result)
	{
		args.VerifyException("splice", "i,i");

		size_t offset = args.GetInt(0);
		size_t length = args.GetInt(1);
		BytesRef slice = new Bytes(BytesRef(this, true), offset, length);
		result->SetObject(slice);
	}
}
