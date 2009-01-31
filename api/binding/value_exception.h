/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_VALUE_EXCEPTION_H_
#define _KR_VALUE_EXCEPTION_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{

#ifndef OS_WIN32
	#pragma GCC visibility push(default)
#endif

	/*
		Class: BoundObject
	*/
	class KROLL_API ValueException : std::exception
	{
	public:
		/*
			Constructor: BoundObject
		*/
		ValueException(SharedValue v);
		~ValueException() throw ();

	public:
		static ValueException FromString(const char* s);
		static ValueException FromString(std::string s);
		static ValueException FromObject(SharedBoundObject o);
		SharedValue GetValue();

	private:
		SharedValue value;
	};

#ifndef OS_WIN32
	#pragma GCC visibility pop
#endif
}

#endif
