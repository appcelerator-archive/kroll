/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2010 Appcelerator, Inc. All Rights Reserved.
 */

#include "thread_pool.h"

namespace kroll
{
	/**
		ThreadPool
	*/

	ThreadPool::ThreadPool(int minCapacity, int maxCapacity, int idleSeconds, int queueMax) :
		minCapacity(minCapacity),
		maxCapacity(maxCapacity),
		idleSeconds(idleSeconds),
		queueMax(queueMax)
	{
		// Spawn initial threads
		for (int c = 0; c < minCapacity; c++)
		{
			PooledThread* thread = new PooledThread(this);
			this->threads.push_back(thread);
			thread->start();
		}
	}

	ThreadPool::~ThreadPool()
	{
		// Wait for all threads to complete
		this->pauseAll();

		// Free all threads
		PooledThreadList::iterator i = this->threads.begin();
		while (i != this->threads.end())
		{
			PooledThread* thread = *i++;
			delete thread;
		}
	}

	bool ThreadPool::start(SharedRunnable target, bool queue)
	{
		PooledThread* thread = this->getIdleThread();
		if (!thread)
		{
			// If no threads are available, queue the job for later
			// if the caller allows it.
			if (queue)
			{
				this->addJob(target);
				return true;
			}

			return false;
		}

		thread->assignJob(target);
		return true;
	}

	void ThreadPool::pauseAll()
	{
		Poco::Mutex::ScopedLock lock(this->threadsMutex);

		PooledThreadList::iterator i = this->threads.begin();
		while (i != this->threads.end())
		{
			PooledThread* thread = *i++;
			if (!thread->idle())
			{
				thread->pause(true);
			}
		}	
	}

	int ThreadPool::availableThreads()
	{
		Poco::Mutex::ScopedLock lock(this->threadsMutex);

		int count = 0;
		PooledThreadList::iterator i = this->threads.begin();
		while (i != this->threads.end())
		{
			PooledThread* thread = *i++;
			if (thread->idle()) count++;
		}

		return count;
	}

	int ThreadPool::totalThreads()
	{
		return this->threads.size();
	}

	void ThreadPool::collect()
	{
		Poco::Mutex::ScopedLock lock(this->threadsMutex);

		PooledThreadList::iterator i = this->threads.begin();
		while (i != this->threads.end())
		{
			PooledThread* thread = *i;
			if (thread->idleTime() > this->idleSeconds)
			{
				thread->shutdown();
				this->threads.erase(i);
			}
			i++;
		}
	}

	bool ThreadPool::canSpawnThread()
	{
		int threadCount = this->totalThreads();
		return (threadCount < this->maxCapacity);
	}

	PooledThread* ThreadPool::getIdleThread()
	{
		Poco::Mutex::ScopedLock lock(this->threadsMutex);

		PooledThreadList::iterator i = this->threads.begin();
		while (i != this->threads.end())
		{
			PooledThread* thread = *i;
			if (thread->idle())
			{
				thread->reserve();
				return thread;
			}
			i++;
		}

		// If we do not find any idle threads, see if we can spawn one.
		if (this->canSpawnThread())
		{
			PooledThread* thread = new PooledThread(this);
			this->threads.push_back(thread);
			thread->start();
			thread->reserve();
			return thread;
		}

		return 0;
	}

	SharedRunnable ThreadPool::getJob()
	{
		Poco::Mutex::ScopedLock lock(this->jobsMutex);
		SharedRunnable job;

		if (!this->pendingJobs.empty())
		{
			job = this->pendingJobs.back();
			this->pendingJobs.pop();
		}
		
		return job;
	}

	void ThreadPool::addJob(SharedRunnable job)
	{
		Poco::Mutex::ScopedLock lock(this->jobsMutex);
		this->pendingJobs.push(job);
	}

	/**
		PooledThread
	*/

	PooledThread::PooledThread(ThreadPool* pool) :
		_state(DEAD),
		sleeping(false),
		pool(pool)
	{
	}

	PooledThread::~PooledThread()
	{
		if (this->alive())
			this->shutdown();
	}

	void PooledThread::start()
	{
		this->_state = IDLE;
		Poco::Thread::start(*this);
	}

	inline void PooledThread::shutdown()
	{
		this->setState(POWERDOWN);
	}

	void PooledThread::assignJob(SharedRunnable job)
	{
		this->job = job;
		this->resume();
	}

	void PooledThread::resume()
	{
		this->setState(WORKING);
		this->wakeup.set();
	}

	inline void PooledThread::pause(bool wait)
	{
		this->setState(PAUSING);
		if (wait)
			this->sleeping.wait();
	}

	inline void PooledThread::reserve()
	{
		this->setState(RESERVED);
	}

	int PooledThread::idleTime()
	{
		if (!this->idle())
			return 0;
		return time(NULL) - this->idleStartTime;
	}

	PooledThread::State PooledThread::state()
	{
		Poco::Mutex::ScopedLock lock(this->stateMutex);
		return this->_state;
	}

	void PooledThread::setState(PooledThread::State state)
	{
		Poco::Mutex::ScopedLock lock(this->stateMutex);
		this->_state = state;
	}

	void PooledThread::work()
	{
		if (!this->job.isNull())
		{
			this->job->run();
			this->job = 0;
		}
		else
		{
			// If there is no assigned job, fetch one from pool.
			this->job = this->pool->getJob();
			if (this->job.isNull())
			{
				// No queued jobs in pool, so sleep.
				this->pause();
			}
		}
	}

	void PooledThread::run()
	{
		bool alive = true;

		while (alive)
		{
			switch (this->state())
			{
				case WORKING:
					this->work();					// Work on some jobs...
					break;
				case POWERDOWN:
					this->setState(DEAD);			// Mark us as a dead thread
					alive = false;
					break;
				case PAUSING:
					time(&this->idleStartTime);		// Start idle time stopwatch
					this->setState(IDLE);			// Enter idle state
					this->sleeping.set();			// Let everyone know we are napping
					this->wakeup.wait();			// Zzzzz
					this->sleeping.reset();			// Reset sleeping event since we are awake now
					break;
				default:
					printf("THREAD POOL ERROR: invalid thread state!\n");
					return;
			}
		}
	}
}
