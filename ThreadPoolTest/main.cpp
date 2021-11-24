#include <iostream>
#include <future>
#include <vector>
#include <thread>
#include <chrono>
#include "../ThreadPool.h"
#define SECOND(n) std::chrono::seconds(n)
#define MILL_SECOND(n) std::chrono::milliseconds(n)

class TimeConsum
{
public:
	TimeConsum() { m_nTimeStart = time(nullptr); };
	operator int64_t&& ()
	{
		return time(nullptr) - m_nTimeStart;
	}

private:
	int64_t m_nTimeStart{ 0 };
};

struct TestMin
{
	template <typename Ty>
	constexpr const int operator()(const Ty& n1, const Ty& n2) const noexcept
	{
		return n1 < n2;
	}
};

void Test()
{
	static const auto function = [](int a, int b, int thread_index)
	{
		std::cout << "This is thread :[" << thread_index << "]" << ",thread id:[" << std::this_thread::get_id() << "]" << std::endl;
		std::this_thread::sleep_for(SECOND(10));
		return a * b;
	};

	std::vector<std::future<int>> vecRes;
	TimeConsum timec;

	for (int i = 0; i < 10; ++i)
	{
		std::this_thread::sleep_for(MILL_SECOND(100));
		vecRes.push_back(std::move(ThreadPool::instance().CommitTask(
			std::bind(function, (rand() % 100), (rand() % 101), i))
		));
	}

	for (size_t i = 0; i < vecRes.size(); ++i)
	{
		std::cout << "Res of :[" << i << "]" << std::endl;
		std::cout << vecRes[i].get() << std::endl;//阻塞调用
	}

	std::cout << "Const time:[" << timec << "]" << ",thread id:[" << std::this_thread::get_id() << "]" << std::endl;
}

void Test_1()
{
	std::vector<std::future<int>> vecRes;
	TimeConsum timec;

	for (int i = 0; i < 10; ++i)
	{
		std::this_thread::sleep_for(MILL_SECOND(100));
		vecRes.push_back(std::move(ThreadPool::instance().CommitTask(
			std::bind(TestMin(), 1, 2))
		));
	}

	for (int i = 0; i < 10; ++i)
	{
		std::this_thread::sleep_for(MILL_SECOND(100));
		vecRes.push_back(std::move(ThreadPool::instance().CommitTask(
			TestMin(), "test1", "test2"
			)
		));
	}

	for (size_t i = 0; i < vecRes.size(); ++i)
	{
		std::cout << "Res of :[" << i << "]" << std::endl;
		std::cout << vecRes[i].get() << std::endl;//阻塞调用
	}

	std::cout << "Const time:[" << timec << "]" << ",thread id:[" << std::this_thread::get_id() << "]" << std::endl;
}

int main()
{
	std::thread t1(Test);
	std::thread t2(Test_1);
	t1.join();
	t2.join();
	return 0;
}

