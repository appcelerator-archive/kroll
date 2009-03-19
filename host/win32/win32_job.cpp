/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <Poco/Semaphore.h>
#include "win32_job.h"

namespace kroll
{
	Win32Job::Win32Job(SharedKMethod method, const ValueList& args, bool wait)
	 : method(method),
	   args(args),
	   wait(wait),
	   return_value(NULL),
	   exception(ValueException(NULL)),
	   semaphore(0, 1)
	{
		// The semaphore starts at 0, meaning that the calling
		// thread can wait for the value to become 0 using wait()
		// and the main thread can call set() after job execution
		// which meets this condition.
	}
	
	Win32Job::~Win32Job()
	{
	}

	void Win32Job::Wait()
	{
		this->semaphore.wait();
	}

	void Win32Job::Execute()
	{
		try
		{
			this->return_value = this->method->Call(this->args);
		}
		catch (ValueException& e)
		{
			this->exception = e;
		}
		catch (Poco::SystemException& e)
		{
			this->exception = ValueException::FromString(e.displayText());
		}
		catch (...)
		{
			this->exception = ValueException::FromString("Unknown Exception from job queue");
		}
		this->semaphore.set();		
		if (!this->wait)
		{
			delete this;
		}
	}

	SharedValue Win32Job::GetResult()
	{
		return this->return_value;
	}

	ValueException Win32Job::GetException()
	{
		return this->exception;
	}
}

