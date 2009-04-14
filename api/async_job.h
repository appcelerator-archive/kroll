/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_ASYNC_JOB_H_
#define _KR_ASYNC_JOB_H_
namespace kroll
{
	class KROLL_API AsyncJob : public StaticBoundObject
	{
		public:
		/*
		 * Create an AsyncJob and initialize its binding-layer properties.
		 */
		AsyncJob(SharedKMethod job);

		/*
		 * Destroy an AsyncJob and release its callbacks.
		 */
		virtual ~AsyncJob();

		/*
		 * Get a SharedPtr to this AsyncJob. The return value of
		 * this method will be NULL after the job finishes executing.
		 */
		SharedPtr<AsyncJob> GetSharedPtr();

		/*
		 * Execute this async job's job on the current thread and block
		 * until it completes.
		 */
		void Execute();

		/**
		 * The result of the execution of this job. On an execution
		 * error and before the job is completed this will be Undefined;
		 */
		SharedValue GetResult();

		/**
		 * The progress of this job, which is a number in
		 * the range [0, 1] where 1 represents fully complete
		 */
		double GetProgress();

		/**
		 * Set progress of this job, which is a number in
		 * the range [0, 1] where 1 represents fully complete.
		 * Calling this method will trigger progress callbacks
		 * unless the second argument is false.
		 */
		void SetProgress(double, bool callbacks = false);

		/**
		 * A built-in progress changed callback. This will be called
		 * in the same situations as KMethod-style progress callbacks
		 */
		virtual void OnProgressChanged() {}

		/**
		 * Add a callback to be executed when the progress of
		 * this job changes
		 */
		void AddProgressCallback(SharedKMethod);

		/**
		 * A built-in completion callback. This will be called  in the
		 *  same situations as KMethod-style completed callbacks
		 */
		virtual void OnCompleted() {};

		/**
		 * Add a callback to be executed when the progress of
		 * this job changes
		 */
		void AddCompletedCallback(SharedKMethod);

		/**
		 * A built-in error callback. This will be called  in the
		 *  same situations as KMethod-style error callbacks
		 */
		virtual void OnError(ValueException& e) {};

		/**
		 * Add a callback to be when an error happes during the
		 * course of this job, whether in the job itself or a callback
		 * related to that job.
		 */
		void AddErrorCallback(SharedKMethod);

		private:
		SharedKMethod job;
		double progress;
		SharedValue result;
		bool hadError;

		SharedPtr<AsyncJob> sharedThis;
		std::vector<SharedKMethod> progressCallbacks;
		std::vector<SharedKMethod> completedCallbacks;
		std::vector<SharedKMethod> errorCallbacks;

		void Error(ValueException&);
		SharedValue DoCallback(SharedKMethod, bool reportErrors=false);
	};
}
#endif
