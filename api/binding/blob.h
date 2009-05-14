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

namespace kroll
{
	/**
	 * An object that represents an arbitrary amount of binary data§
	 */
	class KROLL_API Blob : public StaticBoundObject
	{
	public:


		Blob(char *buffer, int size);
		Blob(const char *buffer, int size);
		Blob(std::string);
		Blob(std::string&);
		virtual ~Blob();

		/**
		 * @return The buffer as a const char *
		 */
		const char* Get() { return buffer; }

		/**
		 * @return the length of the underlying buffer§
		 */
		const int Length () { return length; }

	private:
		char *buffer;
		int length;

		void ToString(const ValueList& args, SharedValue result);
		void Get(const ValueList& args, SharedValue result);
		void Length(const ValueList& args, SharedValue result);

		void IndexOf(const ValueList& args, SharedValue result);
		void LastIndexOf(const ValueList& args, SharedValue result);
		void CharAt(const ValueList& args, SharedValue result);
		void Split(const ValueList& args, SharedValue result);
		void Substring(const ValueList& args, SharedValue result);
		void ToLowerCase(const ValueList& args, SharedValue result);
		void ToUpperCase(const ValueList& args, SharedValue result);
		void Replace(const ValueList& args, SharedValue result);

		void Create(const char *buf, int len);

	};
}

#endif
