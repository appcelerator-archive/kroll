/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <Poco/Semaphore.h>
#include "win32_job.h"

namespace kroll
{
	Win32Job::Win32Job(KMethodRef method, const ValueList& args, bool wait) :
		method(method),
		args(args),
		wait(wait),
		return_value(NULL),
		exception(ValueException(NULL)),
		semaphore(0, 1)
	{
		// The semaphore starts at 0, meaning that the calling
		// thread can wait for the value to become >0 using wait()
		// and the main thread can call set() after job execution
		// which meets this condition.
	}

	Win32Job::~Win32Job()
	{
	}

	void Win32Job::Wait()
	{
		if (this->wait)
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
		catch (std::exception& e)
		{
			this->exception = ValueException::FromString(e.what());
		}
		catch (...)
		{
			this->exception = ValueException::FromString("Unknown Exception from job queue");
		}

		if (this->wait)
			this->semaphore.set();
	}

	KValueRef Win32Job::GetResult()
	{
		return this->return_value;
	}

	bool Win32Job::IsSynchronous()
	{
		return this->wait;
	}

	ValueException Win32Job::GetException()
	{
		return this->exception;
	}

	void Win32Job::PrintException()
	{
		if (this->return_value.isNull())
		{
			SharedString ss = this->exception.GetValue()->DisplayString();
			Logger::Get("Win32Job")->Error("Exception in job queue: %s", ss->c_str());
		}
	}
}

