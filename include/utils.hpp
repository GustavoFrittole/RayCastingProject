#ifndef UTILS_HPP
#define UTILS_HPP
#include<string>
#include<chrono>
#include<glm/vec2.hpp>
#include<glm/mat2x2.hpp>
#include<glm/trigonometric.hpp>

//ONLY_DEBUG
namespace debug
{
	class GameTimer 
	{
	public:
		GameTimer() : tStart(std::chrono::high_resolution_clock::now()){}
		void add_frame();
		int get_frame_rate();
		int get_time_nano() const;
		int reset_timer();
	private:
		int m_frameCounter = 0;
		std::chrono::time_point<std::chrono::high_resolution_clock> tStart;
	};
}
namespace vecMath 
{
	glm::mat2x2 rotation_mat2x2(float angle);
}

#endif