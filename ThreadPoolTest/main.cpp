#include <iostream>
#include <future>
#include <vector>
#include <thread>
#include <chrono>
#include "../ThreadPool.h"

class TimeConsum
{
public:
	TimeConsum() { m_nTimeStart = time(nullptr); };
	operator int64_t && ()
	{
		return time(nullptr) - m_nTimeStart;
	}

private:
	int64_t m_nTimeStart{ 0 };
};

int main()
{
	using std::chrono::operator""s;
	std::vector<std::future<int>> vecRes;
	TimeConsum timec;

	auto function = [](int a, int b, int thread_index)
	{
		std::cout << "This is thread :[" << thread_index << "]" << std::endl;
		std::this_thread::sleep_for(10s);
		return a * b;
	};

	for (int i = 0; i < 100; ++i)
	{
		vecRes.emplace_back(
			std::move(ThreadPool::instance().CommitTask(
				std::bind(function, (rand() % 100), (rand() % 101), i)
			)
			));
	}

	std::this_thread::sleep_for(10s);
	for (size_t i = 0; i < vecRes.size(); ++i)
	{
		std::cout << "Res of :[" << i << "]" << std::endl;
		std::cout << vecRes[i].get() << std::endl;//×èÈûµ÷ÓÃ
	}

	std::cout << "Const time:[" << timec << "]" << std::endl;
}