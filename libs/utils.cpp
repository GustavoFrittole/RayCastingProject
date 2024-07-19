#include "utils.hpp"

int debug::GameTimer::get_frame_rate()
{
	int fr = ((double)m_frameCounter / (double)(std::chrono::high_resolution_clock::now() - tStart).count()) * 1000000000;
	tStart = std::chrono::high_resolution_clock::now();
	m_frameCounter = 0;
	return fr;
}

void debug::GameTimer::add_frame()
{
	m_frameCounter++;
}