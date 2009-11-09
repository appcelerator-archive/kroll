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
#include <Poco/Data/BLOB.h>

namespace kroll
{
	/**
	 * An object that represents an arbitrary amount of binary data§
	 */
	class KROLL_API Blob : public StaticBoundObject
	{
	public:
		/**
		 * If makeCopy is false: create a Blob from a heap-allocated
		 * pointer. The blob * will keep the pointer to this string,
		 * so do not free it afterward. The buffer should be size+1
		 * bytes long and NULL terminated.
		 */
		Blob();
		Blob(char *buffer, long size, bool makeCopy=true);
		Blob(const char *buffer, long size, bool makeCopy=true);
		Blob(std::string);
		Blob(std::string&);
		Blob(Poco::Data::BLOB* blob);
		Blob(long byte);
		virtual ~Blob();

		BlobRef Concat(std::vector<BlobRef>& blobs);
		const char* Get() { return buffer; }
		const long Length() { return length; }

		static BlobRef GlobBlobs(std::vector<BlobRef>& blobs);

	private:
		char* buffer;
		long length;

		void ToString(const ValueList& args, KValueRef result);
		void Get(const ValueList& args, KValueRef result);
		void Length(const ValueList& args, KValueRef result);

		void IndexOf(const ValueList& args, KValueRef result);
		void LastIndexOf(const ValueList& args, KValueRef result);
		void CharAt(const ValueList& args, KValueRef result);
		void ByteAt(const ValueList& args, KValueRef result);
		void Split(const ValueList& args, KValueRef result);
		void Substr(const ValueList& args, KValueRef result);
		void Substring(const ValueList& args, KValueRef result);
		void ToLowerCase(const ValueList& args, KValueRef result);
		void ToUpperCase(const ValueList& args, KValueRef result);
		void Replace(const ValueList& args, KValueRef result);
		void Concat(const ValueList& args, KValueRef result);

		void CreateWithCopy(const char* buffer, long len);
		void CreateWithReference(char* buffer, long len);
	};
}

#endif
