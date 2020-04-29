#pragma once

#include <chrono>

namespace Timer
{
	static auto start_time = std::chrono::high_resolution_clock::now();
	
	inline void start_timer() 
	{
		start_time = std::chrono::high_resolution_clock::now();
	}

	inline double time() 
	{
		auto current_time = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<double, std::chrono::seconds::period>(current_time - start_time).count();
	}
}