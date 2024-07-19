#ifndef UTILS_HPP
#define UTILS_HPP
#include<string>
#include<chrono>

//ONLY_DEBUG
namespace debug
{
	class GameTimer {
	public:
		GameTimer() : tStart(std::chrono::high_resolution_clock::now())
		{}
		void add_frame();

		int get_frame_rate();
	private:
		int m_frameCounter = 0;
		std::chrono::time_point<std::chrono::high_resolution_clock> tStart;
	};
}


#endif