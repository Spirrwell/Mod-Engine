#include "thread.hpp"

const std::thread::id Thread::MAIN_THREAD = std::this_thread::get_id();