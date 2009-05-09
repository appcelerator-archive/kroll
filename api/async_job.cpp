/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "kroll.h"
#include <Poco/Bugcheck.h>
namespace kroll
{
		AsyncJob::AsyncJob(SharedKMethod job) :
			StaticBoundObject(),
			job(job),
			completed(false),
			result(Value::Undefined),
			hadError(false),
			cancelled(false),
			thread(NULL),
			adapter(NULL)
		{
			this->SetProgress(0.0);
			this->SetMethod("getProgress", &AsyncJob::_GetProgress);
			this->SetMethod("cancel", &AsyncJob::_Cancel);
			this->SetMethod("isComplete", &AsyncJob::_IsComplete);

			this->sharedThis = this;
		}

		AsyncJob::~AsyncJob()
		{
			this->progressCallbacks.clear();
			this->completedCallbacks.clear();
			this->errorCallbacks.clear();
			this->job = NULL;
		}

		SharedPtr<AsyncJob> AsyncJob::GetSharedPtr()
		{
			return this->sharedThis;
		}

		void AsyncJob::RunAsynchronously()
		{
			this->adapter = new Poco::RunnableAdapter<AsyncJob>(
				*this,
				&AsyncJob::RunThreadTarget);
			this->thread = new Poco::Thread();
			this->thread->start(*this->adapter);
		}

		void AsyncJob::RunThreadTarget()
		{
			// We are now in a new thread -- on OSX we need to do some 
			// basic bookkeeping for the reference counter, but other
			// than that, everything past here is like executing a job
			// in a synchronous fashion.
#ifdef OS_OSX
			NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif
			this->Run();
#ifdef OS_OSX
			[pool release];
#endif
			try
			{
			//delete this->thread;
			delete this->adapter;
			}
			catch (Poco::BugcheckException& bge)
			{
				std::cout << bge.what() << std::endl;
			}
		}

		void AsyncJob::Run()
		{
			this->result = this->Execute();

			if (!this->hadError)
			{
				this->completed = true;
				this->OnCompleted();

				std::vector<SharedKMethod>::iterator i = this->completedCallbacks.begin();
				while (i != this->completedCallbacks.end())
				{
					this->DoCallback(*i++, true);
				}
			}

			// Make sure that this particular job gets cleaned up.
			this->sharedThis = NULL;
		}

		SharedValue AsyncJob::Execute()
		{
			try
			{
				return this->job->Call(ValueList());
			}
			catch (ValueException& e)
			{
				this->Error(e);
				return Value::Undefined;
			}
		}

		void AsyncJob::Cancel()
		{
			this->cancelled = true;
		}

		double AsyncJob::GetProgress()
		{
			return this->progress;
		}

		void AsyncJob::DoCallback(SharedKMethod method, bool reportErrors)
		{
			Host* host = Host::GetInstance();
			ValueList args;
			args.push_back(Value::NewObject(this->GetSharedPtr()));
			host->InvokeMethodOnMainThread(method, args, false);
		}

		void AsyncJob::SetProgress(double progress, bool callbacks)
		{
			if (progress < 0.0)
			{
				progress = 0.0;
			}
			else if (progress > 1.0)
			{
				progress = 1.0;
			}
			this->progress = progress;

			/*
			 * One can avoid an infinite loop by setting callbacks=false
			 */
			if (callbacks)
			{
				this->OnProgressChanged();

				std::vector<SharedKMethod>::iterator i = this->progressCallbacks.begin();
				while (i != this->progressCallbacks.end())
				{
					this->DoCallback(*i++, true);
				}
			}
		}

		void AsyncJob::Error(ValueException& e)
		{
			this->hadError = true;
			this->OnError(e);

			std::vector<SharedKMethod>::iterator i = this->errorCallbacks.begin();
			while (i != this->errorCallbacks.end())
			{
				this->DoCallback(*i++, false);
			}
		}

		void AsyncJob::AddProgressCallback(SharedKMethod callback)
		{
			this->progressCallbacks.push_back(callback);
		}

		void AsyncJob::AddCompletedCallback(SharedKMethod callback)
		{
			this->completedCallbacks.push_back(callback);
		}

		void AsyncJob::AddErrorCallback(SharedKMethod callback)
		{
			this->errorCallbacks.push_back(callback);
		}

		void AsyncJob::_Cancel(const ValueList& args, SharedValue result)
		{
			this->Cancel();
		}

		void AsyncJob::_GetProgress(const ValueList& args, SharedValue result)
		{
			result->SetDouble(this->GetProgress());
		}

		void AsyncJob::_IsComplete(const ValueList& args, SharedValue result)
		{
			result->SetBool(this->completed);
		}

}
