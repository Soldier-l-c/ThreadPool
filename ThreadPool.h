#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#pragma once

#include <iostream>
#include <thread>
#include <future>
#include <functional>
#include <thread>
#include <mutex>
#include <queue>
#define MAX_THREAD_COUNT 100
#define DEFAULT_THREAD_COUNT std::thread::hardware_concurrency()

#define WAIT_ALL_THREAD_EXIT
#define THREAD_POOL_AUTO_INCREMENT

class ThreadPool
{
public:
	static ThreadPool& instance()
	{
		static ThreadPool pool;
		return pool;
	}

	~ThreadPool()
	{
		m_bRun = false;
		m_cvTask.notify_all();
		for (auto& iter : m_vecPool)
		{
#ifdef WAIT_ALL_THREAD_EXIT
			iter.join();
#else
			iter.detach();
#endif // WAIT_ALL_THREAD_EXIT
		}
	}

	template<typename F, typename... Args>
	auto CommitTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		using RetType = decltype(f(args...));
		auto task = std::make_shared<std::packaged_task<RetType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		{
			std::unique_lock<std::mutex> lock(m_mTaskRun);
			m_queTaskList.emplace([task]() {(*task)(); });
		}

#ifdef THREAD_POOL_AUTO_INCREMENT
		if (m_nIdleThreadNum < 1)
		{
			CreateThread(1);
		}
#endif //THREAD_POOL_AUTO_INCREMENT
		m_cvTask.notify_one();

		auto future = task->get_future();

		return future;
	}

	int32_t PoolNum() const
	{
		return m_vecPool.size();
	}

	int32_t IdleThreadCount() const 
	{
		return m_nIdleThreadNum;
	}

private:
	ThreadPool(int32_t num = DEFAULT_THREAD_COUNT) { CreateThread(num); };
	void CreateThread(int32_t threadNum)
	{
		static const auto threadFun = [this]
		{
			while (m_bRun)
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex>lock(m_mTaskRun);
					m_cvTask.wait(lock, [this] {return !m_bRun || !m_queTaskList.empty(); });

					if (!m_bRun)
					{
						return;
					}

					task = std::move(m_queTaskList.front());
					m_queTaskList.pop();
				}

				m_nIdleThreadNum--;//执行任务时空闲线程数减一
				task();
				m_nIdleThreadNum++;
			}
		};

		while (threadNum-- > 0 && (MAX_THREAD_COUNT > PoolNum()))
		{
			m_vecPool.emplace_back(threadFun);

			//每创建一个线程，空闲线程数加一，
			//保证所有任务执行完 空闲线程数==线程池内线程数
			m_nIdleThreadNum++;
		}
	};

private:
	std::queue<std::function<void()>>m_queTaskList;
	std::vector<std::thread>m_vecPool;

	std::atomic_bool m_bRun{ true };
	std::atomic_int32_t m_nThreadNum{0};
	std::atomic_int32_t m_nIdleThreadNum{ 0 };

	std::mutex m_mTaskRun;
	std::condition_variable m_cvTask;
};

#endif//THREAD_POOL_H