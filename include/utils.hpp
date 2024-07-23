#ifndef UTILS_HPP
#define UTILS_HPP
#include<string>
#include<chrono>


//DEBUG
namespace debug
{
	class GameTimer 
	{
	public:
		GameTimer(){}
		void add_frame();
		int get_frame_rate();
		int get_time_nano() const;
		int reset_timer();
	private:
		int m_frameCounter = 0;
		std::chrono::time_point<std::chrono::high_resolution_clock> tStart;
	};
}
namespace math
{
	struct Mat2x2 
	{
		float params[2][2]{};
		Mat2x2() = default;
	};
	struct Vect2
	{
		float x=0, y=0;
		Vect2() = default;
		Vect2 (float a, float b) : x(a), y(b) {}
		Vect2 operator*(Mat2x2&);
		Vect2 operator+(Vect2&);
		Vect2& operator+=(Vect2&);
		Vect2 operator*(float);
		float lenght() const;
	};

	Mat2x2 rotation_mat2x2(float angle);
}

#endif