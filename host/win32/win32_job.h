/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WIN32_JOB_H
#define _WIN32_JOB_H

#include <kroll/kroll.h>
#include <Poco/Semaphore.h>

namespace kroll
{
	class Win32Job
	{
		public:
			Win32Job(KMethodRef method, const ValueList& args, bool wait);
			virtual ~Win32Job();
			void Lock();
			void Wait();
			void Execute();
			KValueRef GetResult();
			ValueException GetException();
			bool IsSynchronous();
			void PrintException();
	
		private:
			KMethodRef method;
			const ValueList args;
			bool wait;
			KValueRef return_value;
			ValueException exception;
			Poco::Semaphore semaphore;
	};
}

#endif
