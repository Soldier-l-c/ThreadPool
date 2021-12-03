# ThreadPool

## 主要参考
[progschj/ThreadPool](https://github.com/progschj/ThreadPool)

[lzpong/threadpool](https://github.com/lzpong/threadpool)

主要对线程池访问加锁，多线程提交任务可能会有并发问题

## 支持仿函数
```
struct TestMin
{
	template <typename Ty>
	constexpr const int operator()(const Ty& n1, const Ty& n2) const noexcept
	{
		return n1 < n2;
	}
};

vecRes.push_back(std::move(ThreadPool::instance().CommitTask(
	std::bind(TestMin(), (rand() % 100), (rand() % 101)))
));

vecRes.push_back(std::move(ThreadPool::instance().CommitTask(
	TestMin<std::string>(), "test1", "test2"
	)
));
```
## 支持不定参数
```
static const auto function = [](int a, int b, int thread_index)
{
	std::cout << "This is thread :[" << thread_index << "]" << ",thread id:[" << std::this_thread::get_id() << "]" << std::endl;
	std::this_thread::sleep_for(SECOND(10));
	return a * b;
};

vecRes.push_back(std::move(ThreadPool::instance().CommitTask(
			std::bind(function, (rand() % 100), (rand() % 101), 0))
	));
```
