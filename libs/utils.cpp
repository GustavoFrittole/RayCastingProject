#include "utils.hpp"
#include<cmath>
#include<thread>

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

math::Mat2x2 math::rotation_mat2x2(float angle)
{
	float angRads = angle;
	return math::Mat2x2{std::cos(angRads), -std::sin(angRads),
		std::sin(angRads), std::cos(angRads)};
}

math::Vect2 math::Vect2::operator*(math::Mat2x2& mat2x2)
{
	return math::Vect2{ x * mat2x2.params[0][0] + y * mat2x2.params[0][1], x * mat2x2.params[1][0] + y * mat2x2.params[1][1] };
}

math::Vect2& math::Vect2::operator*=(math::Mat2x2& mat2x2)
{
	this->x = x * mat2x2.params[0][0] + y * mat2x2.params[0][1];
	this->y = x * mat2x2.params[1][0] + y * mat2x2.params[1][1];
	return *this;
}

float math::Vect2::lenght() const
{
	return std::sqrt(x * x + y * y);
}

math::Vect2 math::Vect2::operator+(Vect2& other)
{
	return { x + other.x, y + other.y };
}
math::Vect2& math::Vect2::operator+=(Vect2& other)
{
	this->x += other.x;
	this->y += other.y;
	return *this;
}
math::Vect2 math::Vect2::operator*(float scal)
{
	return { x * scal, y * scal };
}

int get_thread_number()
{
	int t = std::thread::hardware_concurrency();
	return t == 0 ? 1 : t;
}