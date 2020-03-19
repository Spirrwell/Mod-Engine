#ifndef STATE_HPP
#define STATE_HPP

template< typename T >
struct State
{
	void SetOldToNow() { old = now; }

	T old = {};
	T now = {};
};

#endif // STATE_HPP