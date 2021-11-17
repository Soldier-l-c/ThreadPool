#include <iostream>
#include "../ThreadPool.h"
int main()
{
	auto res = ThreadPool::instance().CommitTask([](int a, int b, int c) {return a * b * c; }, 1, 2, 4);
	std::cout << res.get() << std::endl;//×èÈûµ÷ÓÃ
}