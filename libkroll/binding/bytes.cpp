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
	Bytes::Bytes() : StaticBoundObject("Bytes")
	{
		CreateWithCopy(NULL, 0);
	}

	Bytes::Bytes(char* bufferIn, long length, bool makeCopy) :
		StaticBoundObject("Bytes")
	{
		if (makeCopy)
		{
			CreateWithCopy(bufferIn, length);
		}
		else
		{
			CreateWithReference(bufferIn, length);
		}
	}

	Bytes::Bytes(const char* bufferIn, long length, bool makeCopy) :
		StaticBoundObject("Bytes")
	{
		CreateWithCopy(bufferIn, length);
	}

	Bytes::Bytes(std::string str) : StaticBoundObject("Bytes")
	{
		CreateWithCopy(str.c_str(), str.length());
	}

	Bytes::Bytes(std::string& str) : StaticBoundObject("Bytes")
	{
		CreateWithCopy(str.c_str(), str.length());
	}

	Bytes::Bytes(Poco::Data::BLOB *bytes) : StaticBoundObject("Bytes")
	{
		CreateWithReference((char *)&(bytes->content()[0]), bytes->size());
	}

	Bytes::Bytes(long byte) : StaticBoundObject("Bytes")
	{
		CreateWithCopy((char *) &byte, 1);
	}

	void Bytes::CreateWithCopy(const char* bufferIn, long length)
	{
		if (length > 0)
		{
			// Store the buffer with a null terminator so
			// that we can use it like a string later on.
			char* bufferCopy = new char[length + 1];
			memcpy(bufferCopy, bufferIn, length);
			bufferCopy[length] = '\0';
			this->CreateWithReference(bufferCopy, length);
		}
		else
		{
			this->CreateWithReference(NULL, length);
		}
	}

	void Bytes::CreateWithReference(char* buffer, long length)
	{
		this->buffer = buffer;
		this->length = length;

		/**
		 * @tiapi(method=True,name=Bytes.toString,since=0.3)
		 * @tiapi Return a string representation of a bytes
		 * @tiresult[String] This bytes as a String
		 */
		this->SetMethod("toString", &Bytes::ToString);

		// Mimic some string operations to make it more
		// friendly when using a Bytes in JavaScript.
		/**
		 * @tiapi(method=True,name=Bytes.indexOf,since=0.3)
		 * @tiapi Return the index of a String within this Bytes
		 * @tiarg[String, needle] The String to search for
		 * @tiresult[Number] The integer index of the String or -1 if not found
		 */
		this->SetMethod("indexOf", &Bytes::IndexOf);

		/**
		 * @tiapi(method=True,name=Bytes.lastIndexOf,since=0.3)
		 * @tiapi Return the last index of a String within this Bytes
		 * @tiarg[String, needle] The String to search for
		 * @tiresult[Number] The last integer index of the String or -1 if not found
		 */
		this->SetMethod("lastIndexOf", &Bytes::LastIndexOf);

		/**
		 * @tiapi(method=True,name=Bytes.charAt,since=0.3)
		 * @tiapi Return a character representing a byte at the given index in a Bytes
		 * @tiarg[Number, index] The index to look for a character at
		 * @tiresult[String] A String containing a character representing the byte at the given index
		 */
		this->SetMethod("charAt", &Bytes::CharAt);
		
		/**
		 * @tiapi(method=True,name=Bytes.byteAt,since=0.7)
		 * @tiapi Return the character code (or byte value) at the given index in a Bytes
		 * @tiarg[Number, index] The index to look for a character code at
		 * @tiresult[Number] The character code (or byte value) at the given index
		 */
		this->SetMethod("byteAt", &Bytes::ByteAt);

		/**
		 * @tiapi(method=True,name=Bytes.split,since=0.3)
		 * @tiapi Split a bytes as if it were a string given a delimiter. 
		 * @tiapi This method returns empty matches. For instance:
		 * @tiapi <pre><code>"abc,def,,".split(",") --> ['abc', 'def', '', '']</code></pre>
		 * @tiarg[String, delimiter] The index to look for a character at
		 * @tiarg[Number, limit, optional=True] The maximum number of matches to return 
		 * @tiresult[Array<String>] A array containing the segments
		 */
		this->SetMethod("split", &Bytes::Split);

		/**
		 * @tiapi(method=True,name=Bytes.substring,since=0.3)
		 * @tiapi Return a substring of a Bytes given a start index and end index
		 * @tiapi If no end index is given, return all characters from the start index
		 * @tiapi to the end of the string. If startIndex > endIndex, the indexes are swapped.
		 * @tiarg[Number, startIndex] The starting index
		 * @tiarg[Number, endIndex, optional=True] The ending index
		 * @tiresult[String] The substring between startIndex and endIndex
		 */
		this->SetMethod("substring", &Bytes::Substring);

		/**
		 * @tiapi(method=True,name=Bytes.substr,since=0.3)
		 * @tiapi Return a substring of a Bytes given a start index and a length
		 * @tiapi If no length is given, all characters from the start to the
		 * @tiapi end of the string are returned.
		 * @tiarg[Number, startIndex] The starting index
		 * @tiarg[Number, length, optional=True] The length of the substring
		 * @tiresult[String] The substring between startIndex and the given length
		 */
		this->SetMethod("substr", &Bytes::Substr);

		/**
		 * @tiapi(method=True,name=Bytes.toLowerCase,since=0.3)
		 * @tiapi Convert characters in the Bytes to lower case as if it were a string.
		 * @tiresult[String] The resulting String
		 */
		this->SetMethod("toLowerCase", &Bytes::ToLowerCase);

		/**
		 * @tiapi(method=True,name=Bytes.toUpperCase,since=0.3)
		 * @tiapi Convert characters in the Bytes to upper case as if it were a string.
		 * @tiresult[String] The resulting String
		 */
		this->SetMethod("toUpperCase", &Bytes::ToUpperCase);
		
		/**
		 * @tiapi(method=True,name=Bytes.concat,since=0.7)
		 * @tiapi Concatenate multiple Bytes and/or strings into one Bytes
		 * @tiresult[Bytes] The resulting Bytes
		 */
		this->SetMethod("concat", &Bytes::Concat);
		
		/**
		 * @tiapi(property=True,name=Bytes.length,since=0.3) The number of bytes in this bytes
		 */
		this->Set("length", Value::NewInt(length));
	}

	Bytes::~Bytes()
	{
		if (this->buffer!=NULL)
		{
			delete [] this->buffer;
		}
		
		this->buffer = NULL;
		this->length = 0;
	}

	void Bytes::ToString(const ValueList& args, KValueRef result)
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

	void Bytes::Length(const ValueList& args, KValueRef result)
	{
		result->SetInt(length);
	}

	void Bytes::IndexOf(const ValueList& args, KValueRef result)
	{
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/indexOf
		args.VerifyException("Bytes.indexOf", "s,?i");

		if (this->length <= 0)
		{
			result->SetInt(-1);
		}
		else
		{
			std::string target = this->buffer;
			std::string needle = args.at(0)->ToString();
			long start = 0;
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

	void Bytes::LastIndexOf(const ValueList& args, KValueRef result)
	{
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/lastIndexOf
		args.VerifyException("Bytes.lastIndexOf", "s,?i");

		if (this->length <= 0)
		{
			result->SetInt(-1);
		}
		else
		{
			std::string target = this->buffer;
			std::string needle = args.at(0)->ToString();
			long start = target.size() + 1;
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

	void Bytes::CharAt(const ValueList& args, KValueRef result)
	{
		// https://developer.mozilla.org/en/core_javascript_1.5_reference/global_objects/string/charat
		args.VerifyException("Bytes.charAt", "n");
		long  position = args.at(0)->ToInt();

		char buf[2] = {'\0', '\0'};
		if (position >= 0 && position < this->length)
		{
			buf[0] = this->buffer[position];
		}
		result->SetString(buf);
	}
	
	void Bytes::ByteAt(const ValueList& args, KValueRef result)
	{
		args.VerifyException("Bytes.byteAt", "n");
		long position = args.at(0)->ToInt();
		
		if (position >= 0 && position < this->length)
		{
			result->SetInt(static_cast<unsigned char>(this->buffer[position]));
		}
	}

	void Bytes::Split(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/split
		// Except support for regular expressions
		args.VerifyException("Bytes.split", "?s,i");

		KListRef list = new StaticBoundList();
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

	void Bytes::Substr(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/substr
		args.VerifyException("Bytes.substr", "i,?i");
		std::string target = "";
		if (this->length > 0)
		{
			target = this->buffer;
		}

		long start = args.GetInt(0);
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
			start = this->length + start;
		}

		long length = this->length - start;
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

	void Bytes::Substring(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/substring
		args.VerifyException("Bytes.substring", "i,?i");
		std::string target = "";
		if (this->length > 0)
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

	void Bytes::ToLowerCase(const ValueList& args, KValueRef result)
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

	void Bytes::ToUpperCase(const ValueList& args, KValueRef result)
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
	
	BytesRef Bytes::Concat(std::vector<BytesRef>& bytes)
	{
		if (bytes.size() == 0)
			return BytesRef(this, true);

		long size = this->Length();
		for (size_t i = 0; i < bytes.size(); i++)
		{
			size += bytes.at(i)->Length();
		}

		char* buffer = new char[size+1];
		buffer[size] = '\0';

		char* current = buffer;
		memcpy(current, this->Get(), this->Length());
		current += this->Length();
		
		for (size_t i = 0; i < bytes.size(); i++)
		{
			BytesRef bytesObject(bytes.at(i));
			if (bytesObject->Length() > 0)
			{
				memcpy(current, bytesObject->Get(), bytesObject->Length());
				current += bytesObject->Length();
			}
		}

		return new Bytes(buffer, size, false);
	}

	void Bytes::Concat(const ValueList& args, KValueRef result)
	{
		std::vector<BytesRef> bytes;
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
				bytes.push_back(new Bytes(args.GetString(i)));
			}
		}

		BytesRef newBytes = this->Concat(bytes);
		result->SetObject(newBytes);
	}

	/*static*/
	BytesRef Bytes::GlobBytes(std::vector<BytesRef>& bytes)
	{
		BytesRef bytesObject(new Bytes());
		return bytesObject->Concat(bytes);
	}
}
