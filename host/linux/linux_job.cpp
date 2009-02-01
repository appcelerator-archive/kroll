/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <Poco/Semaphore.h>
#include "linux_job.h"

namespace kroll
{
	LinuxJob::LinuxJob(SharedBoundMethod method, const ValueList& args)
	 : method(method),
	   args(args),
	   return_value(NULL),
	   exception(ValueException(NULL)),
	   semaphore(0, 1)
	{
		// The semaphore starts at 0, meaning that the calling
		// thread can wait for the value to become 0 using wait()
		// and the main thread can call set() after job execution
		// which meets this condition.
	}

	void LinuxJob::Wait()
	{
		this->semaphore.wait();
	}

	void LinuxJob::Execute()
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
			std::cout << "caught poco exception " << e.displayText() << std::endl;
		}
		catch (...)
		{
			this->exception =
			  ValueException::FromString("Unknown Exception from job queue");
		}
		this->semaphore.set();
	}

	SharedValue LinuxJob::GetResult()
	{
		return this->return_value;
	}

	ValueException LinuxJob::GetException()
	{
		return this->exception;
	}
}

