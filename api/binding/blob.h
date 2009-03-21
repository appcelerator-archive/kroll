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

	};
}

#endif
