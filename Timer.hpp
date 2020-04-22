#pragma once

#include <chrono>

class Timer
{
private:
	double _delta_time;

	std::chrono::time_point<std::chrono::high_resolution_clock> timer_start;
	std::chrono::time_point<std::chrono::high_resolution_clock> timer_stop;
public:

	inline void start_timer() 
	{
		timer_start = std::chrono::high_resolution_clock::now();
	}

	inline void stop_timer()
	{
		timer_stop = std::chrono::high_resolution_clock::now();
		_delta_time = 0.000000001 * std::chrono::duration_cast<std::chrono::microseconds>(timer_stop - timer_start).count();
	}

	inline double delta_time() 
	{
		return _delta_time;
	}
};