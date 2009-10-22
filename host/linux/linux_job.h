
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _LINUX_JOB_H
#define _LINUX_JOB_H

#include <kroll/kroll.h>
#include <Poco/Semaphore.h>

namespace kroll
{
	class LinuxJob
	{
		public:
			LinuxJob(KMethodRef method, const ValueList& args, bool synchronous);
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
			bool synchronous;
			KValueRef return_value;
			ValueException exception;
			Poco::Semaphore semaphore;
	};
}

#endif
