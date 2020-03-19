#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <chrono>

struct Clock
{
	void Start() {
		StartTime = std::chrono::high_resolution_clock::now();
	}

	template < typename T = float, typename Rep = std::chrono::seconds >
	T Duration() const {
		
		auto delta = std::chrono::duration_cast< std::chrono::duration< T, typename Rep::period > >( std::chrono::high_resolution_clock::now() - StartTime );
		return delta.count();
	}

private:
	std::chrono::high_resolution_clock::time_point StartTime = {};
};

#endif // CLOCK_HPP