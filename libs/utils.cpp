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

glm::mat2x2 vecMath::rotation_mat2x2(float angle)
{
	float angRads = glm::radians(angle);
	return glm::mat2x2(glm::cos(angRads), -glm::sin(angRads),
		glm::sin(angRads), glm::cos(angRads));
}

int debug::GameTimer::get_time_nano() const
{
	return (std::chrono::high_resolution_clock::now() - tStart).count();
}

int debug::GameTimer::reset_timer()
{
	int dt = (std::chrono::high_resolution_clock::now() - tStart).count();
	tStart = std::chrono::high_resolution_clock::now();
	return dt;
}