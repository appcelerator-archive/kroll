/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2010 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_THREAD_POOL_H_
#define _KR_THREAD_POOL_H_

#include <ctime>

#include <list>
#include <queue>

#include <Poco/SharedPtr.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Mutex.h>
#include <Poco/Event.h>

#include "kroll.h"

namespace kroll
{
	typedef Poco::SharedPtr<Poco::Runnable> SharedRunnable;
	typedef std::queue<SharedRunnable> PoolJobQueue;

	class PooledThread;
	typedef std::list<PooledThread*> PooledThreadList;

	class KROLL_API ThreadPool
	{
		friend class PooledThread;

	public:
		ThreadPool(
			int minCapacity = 2,
			int maxCapacity = 10,
			int idleSeconds = 60,
			int queueMax = -1
		);
		virtual ~ThreadPool();

		bool start(SharedRunnable target, bool queue = true);
		void pauseAll();

		int availableThreads();
		int totalThreads();
		void collect();

	protected:
		virtual bool canSpawnThread();

		int minCapacity, maxCapacity;
		int idleSeconds;
		int queueMax;

	private:
		PooledThread* getIdleThread();

		SharedRunnable getJob();
		void addJob(SharedRunnable job);

		PooledThreadList threads;
		Poco::Mutex threadsMutex;

		PoolJobQueue pendingJobs;
		Poco::Mutex jobsMutex;
	};

	class KROLL_API PooledThread : public Poco::Thread, public Poco::Runnable
	{
	public:
		PooledThread(ThreadPool* pool);
		virtual ~PooledThread();

		enum State {
			IDLE,
			PAUSING,
			RESERVED,
			WORKING,
			POWERDOWN,
			DEAD
		};

		void start();
		void shutdown();
		void assignJob(SharedRunnable job);
		void resume();
		void pause(bool wait=false);
		void reserve();

		bool idle() { return state() == IDLE; }
		bool working() { return state() == WORKING; }
		bool alive() { return state() != DEAD; }

		int idleTime();

		void run();

	private:
		State state();
		void setState(State state);
		void work();

		State _state;
		Poco::Mutex stateMutex;
		Poco::Event wakeup, sleeping;
		SharedRunnable job;
		ThreadPool* pool;
		time_t idleStartTime;
	};
}

#endif