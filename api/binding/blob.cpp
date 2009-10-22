/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "blob.h"
#include <cstring>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Data/BLOB.h>

namespace kroll
{
	Blob::Blob() : StaticBoundObject("Blob")
	{
		CreateWithCopy(NULL, 0);
	}

	Blob::Blob(char *bufferIn, int len, bool makeCopy) :
		StaticBoundObject("Blob")
	{
		if (makeCopy)
		{
			CreateWithCopy(bufferIn, len);
		}
		else
		{
			CreateWithReference(bufferIn, len);
		}
	}

	Blob::Blob(const char *bufferIn, int len, bool makeCopy) :
		StaticBoundObject("Blob")
	{
		CreateWithCopy(bufferIn, len);
	}

	Blob::Blob(std::string str) : StaticBoundObject("Blob")
	{
		CreateWithCopy(str.c_str(), str.length());
	}

	Blob::Blob(std::string& str) : StaticBoundObject("Blob")
	{
		CreateWithCopy(str.c_str(), str.length());
	}

	Blob::Blob(Poco::Data::BLOB *blob) : StaticBoundObject("Blob")
	{
		CreateWithReference((char *)&(blob->content()[0]), blob->size());
	}
	
	Blob::Blob(int byte) : StaticBoundObject("Blob")
	{
		CreateWithCopy((char *) &byte, 1);
	}
	
	void Blob::CreateWithCopy(const char *bufferIn, int length)
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

	void Blob::CreateWithReference(char* buffer, int length)
	{
		this->buffer = buffer;
		this->length = length;

		/**
		 * @tiapi(method=True,name=Blob.toString,since=0.3)
		 * @tiapi Return a string representation of a blob
		 * @tiresult[String] This blob as a String
		 */
		this->SetMethod("toString", &Blob::ToString);

		/**
		 * @tiapi(method=True,name=Blob.get,since=0.3)
		 * @tiapi Return a VoidPtr representation of a Blob
		 * @tiresult[VoidPtr] This blob as a VoidPtr
		 */
		this->SetMethod("get", &Blob::Get);

		// mimic some string operations to make it more 
		// friendly when using a Blob in Javascript
		/**
		 * @tiapi(method=True,name=Blob.indexOf,since=0.3)
		 * @tiapi Return the index of a String within this Blob
		 * @tiarg[String, needle] The String to search for
		 * @tiresult[Number] The integer index of the String or -1 if not found
		 */
		this->SetMethod("indexOf", &Blob::IndexOf);

		/**
		 * @tiapi(method=True,name=Blob.lastIndexOf,since=0.3)
		 * @tiapi Return the last index of a String within this Blob
		 * @tiarg[String, needle] The String to search for
		 * @tiresult[Number] The last integer index of the String or -1 if not found
		 */
		this->SetMethod("lastIndexOf", &Blob::LastIndexOf);

		/**
		 * @tiapi(method=True,name=Blob.charAt,since=0.3)
		 * @tiapi Return a character representing a byte at the given index in a Blob
		 * @tiarg[Number, index] The index to look for a character at
		 * @tiresult[String] A String containing a character representing the byte at the given index
		 */
		this->SetMethod("charAt", &Blob::CharAt);
		
		/**
		 * @tiapi(method=True,name=Blob.byteAt,since=0.7)
		 * @tiapi Return the character code (or byte value) at the given index in a Blob
		 * @tiarg[Number, index] The index to look for a character code at
		 * @tiresult[Number] The character code (or byte value) at the given index
		 */
		this->SetMethod("byteAt", &Blob::ByteAt);

		/**
		 * @tiapi(method=True,name=Blob.split,since=0.3)
		 * @tiapi Split a blob as if it were a string given a delimiter. 
		 * @tiapi This method returns empty matches. For instance:
		 * @tiapi <pre><code>"abc,def,,".split(",") --> ['abc', 'def', '', '']</code></pre>
		 * @tiarg[String, delimiter] The index to look for a character at
		 * @tiarg[Number, limit, optional=True] The maximum number of matches to return 
		 * @tiresult[Array<String>] A array containing the segments
		 */
		this->SetMethod("split", &Blob::Split);

		/**
		 * @tiapi(method=True,name=Blob.substring,since=0.3)
		 * @tiapi Return a substring of a Blob given a start index and end index
		 * @tiapi If no end index is given, return all characters from the start index
		 * @tiapi to the end of the string. If startIndex > endIndex, the indexes are swapped.
		 * @tiarg[Number, startIndex] The starting index
		 * @tiarg[Number, endIndex, optional=True] The ending index
		 * @tiresult[String] The substring between startIndex and endIndex
		 */
		this->SetMethod("substring", &Blob::Substring);

		/**
		 * @tiapi(method=True,name=Blob.substr,since=0.3)
		 * @tiapi Return a substring of a Blob given a start index and a length
		 * @tiapi If no length is given, all characters from the start to the
		 * @tiapi end of the string are returned.
		 * @tiarg[Number, startIndex] The starting index
		 * @tiarg[Number, length, optional=True] The length of the substring
		 * @tiresult[String] The substring between startIndex and the given length
		 */
		this->SetMethod("substr", &Blob::Substr);

		/**
		 * @tiapi(method=True,name=Blob.toLowerCase,since=0.3)
		 * @tiapi Convert characters in the Blob to lower case as if it were a string.
		 * @tiresult[String] The resulting String
		 */
		this->SetMethod("toLowerCase", &Blob::ToLowerCase);

		/**
		 * @tiapi(method=True,name=Blob.toUpperCase,since=0.3)
		 * @tiapi Convert characters in the Blob to upper case as if it were a string.
		 * @tiresult[String] The resulting String
		 */
		this->SetMethod("toUpperCase", &Blob::ToUpperCase);
		
		/**
		 * @tiapi(method=True,name=Blob.concat,since=0.7)
		 * @tiapi Concatenate multiple Blob and/or strings into one Blob
		 * @tiresult[Blob] The resulting Blob
		 */
		this->SetMethod("concat", &Blob::Concat);
		
		/**
		 * @tiapi(property=True,name=Blob.length,since=0.3) The number of bytes in this blob
		 */
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

	void Blob::ToString(const ValueList& args, KValueRef result)
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

	void Blob::Get(const ValueList& args, KValueRef result)
	{
		result->SetVoidPtr(buffer);
	}

	void Blob::Length(const ValueList& args, KValueRef result)
	{
		result->SetInt(length);
	}

	void Blob::IndexOf(const ValueList& args, KValueRef result)
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

	void Blob::LastIndexOf(const ValueList& args, KValueRef result)
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

	void Blob::CharAt(const ValueList& args, KValueRef result)
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
	
	void Blob::ByteAt(const ValueList& args, KValueRef result)
	{
		args.VerifyException("Blob.byteAt", "n");
		int position = args.at(0)->ToInt();
		
		if (position >= 0 && position < this->length)
		{
			result->SetInt(static_cast<unsigned char>(this->buffer[position]));
		}
	}

	void Blob::Split(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/split
		// Except support for regular expressions
		args.VerifyException("Blob.split", "?s,i");

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
		int limit = INT_MAX;
		if (args.size() > 1)
		{
			limit = args.GetInt(1);
		}

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

	void Blob::Substr(const ValueList& args, KValueRef result)
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
			start = this->length + start;
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

	void Blob::Substring(const ValueList& args, KValueRef result)
	{
		// This method now follows the spec located at:
		// https://developer.mozilla.org/en/Core_JavaScript_1.5_Reference/Global_Objects/String/substring
		args.VerifyException("Blob.substring", "i,?i");
		std::string target = "";
		if (this->length > 0)
		{
			target = this->buffer;
		}

		int indexA = args.GetInt(0);
		if (indexA < 0)
			indexA = 0;
		if (indexA > (int) target.size())
			indexA = target.size();

		if (args.size() < 2)
		{
			std::string r = target.substr(indexA);
			result->SetString(r);
		}
		else
		{
			int indexB = args.GetInt(1);
			if (indexB < 0)
				indexB = 0;
			if (indexB > (int) target.size())
				indexB = target.size();

			if (indexA == indexB)
			{
				result->SetString("");
				return;
			}
			if (indexA > indexB)
			{
				int temp = indexA;
				indexA = indexB;
				indexB = temp;
			}
			std::string r = target.substr(indexA, indexB - indexA);
			result->SetString(r);
		}
	}

	void Blob::ToLowerCase(const ValueList& args, KValueRef result)
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

	void Blob::ToUpperCase(const ValueList& args, KValueRef result)
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
	
	BlobRef Blob::Concat(std::vector<BlobRef>& blobs)
	{
		if (blobs.size() == 0) {
			this->duplicate();
			return this;
		}
		
		int size = this->Length();
		for (size_t i = 0; i < blobs.size(); i++)
		{
			size += blobs.at(i)->Length();
		}

		char* buffer = new char[size+1];
		buffer[size] = '\0';

		char* current = buffer;
		memcpy(current, this->Get(), this->Length());
		current += this->Length();
		
		for (size_t i = 0; i < blobs.size(); i++)
		{
			BlobRef blob = blobs.at(i);
			if (blob->Length() > 0)
			{
				memcpy(current, blob->Get(), blob->Length());
				current += blob->Length();
			}
		}
		
		return new Blob(buffer, size, false);
	}
	
	void Blob::Concat(const ValueList& args, KValueRef result)
	{
		std::vector<BlobRef> blobs;
		for (size_t i = 0; i < args.size(); i++)
		{
			if (args.at(i)->IsObject())
			{
				BlobRef blob = args.GetObject(i).cast<Blob>();
				if (!blob.isNull())
				{
					blobs.push_back(blob);
				}
			}
			else if (args.at(i)->IsString())
			{
				blobs.push_back(new Blob(args.GetString(i)));
			}
		}
		
		BlobRef newBlob = this->Concat(blobs);
		result->SetObject(newBlob);
	}

	/*static*/
	BlobRef Blob::GlobBlobs(std::vector<BlobRef>& blobs)
	{
		BlobRef blob = new Blob();
		blob = blob->Concat(blobs);
		return blob;
	}
}
