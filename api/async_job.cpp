/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "kroll.h"

namespace kroll
{
		AsyncJob::AsyncJob(SharedKMethod job) :
			StaticBoundObject(),
			job(job),
			result(Value::Undefined),
			hadError(false)
		{
			this->SetProgress(0.0);
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

		void AsyncJob::Execute()
		{
			if (job.isNull())
				return;

			this->result = DoCallback(job);
			this->SetProgress(1.0);

			if (this->hadError)
				return;

			this->OnCompleted();
			std::vector<SharedKMethod>::iterator i = this->completedCallbacks.begin();
			while (i != this->completedCallbacks.end())
			{
				this->DoCallback(*i++, true);
			}

			// Make sure that this particular job gets cleaned up.
			this->sharedThis = NULL;
		}

		double AsyncJob::GetProgress()
		{
			return this->progress;
		}

		SharedValue AsyncJob::DoCallback(SharedKMethod method, bool reportErrors)
		{
			try
			{
				Host* host = Host::GetInstance();
				ValueList args;
				args.push_back(Value::NewObject(this->GetSharedPtr()));
				return host->InvokeMethodOnMainThread(method, args);
			}
			catch (ValueException& e)
			{
				if (reportErrors)
					this->Error(e);
				return Value::Undefined;
			}
		}

		void AsyncJob::SetProgress(double progress, bool callbacks)
		{
			if (progress < 0.0)
				progress = 0.0;
			if (progress > 1.0)
				progress = 1.0;

			this->progress = progress;
			this->Set("progress", Value::NewDouble(this->progress));

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
}
