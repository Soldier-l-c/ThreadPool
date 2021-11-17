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
#define MAX_THREAD_COUNT 20

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
			iter.detach();
		}
	}

	template<typename F, typename... Args>
	auto CommitTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		using RetType = decltype(f(args...));
		auto task = std::make_shared<std::packaged_task<RetType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);
		m_queTaskList.emplace([task]() {(*task)(); });

		CreateThread(1);
		m_cvTask.notify_one();

		auto future = task->get_future();

		return future;
	}

	int32_t PoolNum() 
	{
		return m_vecPool.size();
	}

private:
	ThreadPool() {};
	void CreateThread(int32_t threadNum)
	{
		while (threadNum-- > 0 && (MAX_THREAD_COUNT > PoolNum()))
		{
			m_vecPool.emplace_back([this] 
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
						task();
					}
				});
		}
	};

private:
	std::queue<std::function<void()>>m_queTaskList;
	std::vector<std::thread>m_vecPool;

	std::atomic_bool m_bRun{ true };
	std::atomic_int32_t m_nThreadNum{0};

	std::mutex m_mTaskRun;
	std::condition_variable m_cvTask;
};

#endif//THREAD_POOL_H