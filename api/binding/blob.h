/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_BLOB_OBJECT_H_
#define _KR_BLOB_OBJECT_H_

#include "../kroll.h"
#include <vector>
#include <string>
#include <map>
#include <cstring>

namespace Poco
{
	namespace Data
	{
		class BLOB;
	}
}

namespace kroll
{
	/**
	 * An object that represents an arbitrary amount of binary data§
	 */
	class KROLL_API Blob : public StaticBoundObject
	{
	public:
		/**
		 * if makeCopy is false: create a Blob from a heap-allocated
		 * pointer. The blob * will keep the pointer to this string,
		 * so do not free it afterward. The buffer should be size+1
		 * bytes long and NULL terminated.
		 */
		Blob();
		Blob(char *buffer, int size, bool makeCopy=true);
		Blob(const char *buffer, int size, bool makeCopy=true);
		Blob(std::string);
		Blob(std::string&);
		Blob(Poco::Data::BLOB* blob);
		Blob(int byte);
		
		virtual ~Blob();
		void SetupBinding();
		AutoBlob Concat(std::vector<AutoBlob>& blobs);
		
		static AutoBlob GlobBlobs(std::vector<AutoBlob>& blobs);
		/**
		 * @return The buffer as a const char *
		 */
		const char* Get() { return buffer; }

		/**
		 * @return the length of the underlying buffer§
		 */
		const int Length() { return length; }

	private:
		char *buffer;
		int length;

		void ToString(const ValueList& args, SharedValue result);
		void Get(const ValueList& args, SharedValue result);
		void Length(const ValueList& args, SharedValue result);

		void IndexOf(const ValueList& args, SharedValue result);
		void LastIndexOf(const ValueList& args, SharedValue result);
		void CharAt(const ValueList& args, SharedValue result);
		void ByteAt(const ValueList& args, SharedValue result);
		void Split(const ValueList& args, SharedValue result);
		void Substr(const ValueList& args, SharedValue result);
		void Substring(const ValueList& args, SharedValue result);
		void ToLowerCase(const ValueList& args, SharedValue result);
		void ToUpperCase(const ValueList& args, SharedValue result);
		void Replace(const ValueList& args, SharedValue result);
		void Concat(const ValueList& args, SharedValue result);

		void CreateWithCopy(const char *buf, int len);
		void CreateWithReference(char *buf, int len);
	};
}

#endif
