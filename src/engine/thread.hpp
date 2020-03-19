#ifndef THREAD_HPP
#define THREAD_HPP

#include <thread>

class Thread
{
	static const std::thread::id MAIN_THREAD;

public:
	static std::thread::id GetMainThreadId() { return MAIN_THREAD; }
};

#endif // THREAD_HPP