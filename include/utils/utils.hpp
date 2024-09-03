#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <chrono>

#define PI 3.14159265359f
//DEBUG
namespace debug
{
	class GameTimer 
	{
	public:
		GameTimer(){}
		void add_frame();
		int get_frame_rate();
		int get_frame_rate_noreset() const { return m_frameCounter; };
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
		Mat2x2() = default;
		float params[2][2]{ { 0, 0 }, { 0, 0 } };
		Mat2x2(float, float, float, float);
	};
	struct Vect2
	{
		float x=0, y=0;
		Vect2() = default;
		Vect2 (float a, float b) : x(a), y(b) {}
		Vect2 operator*(const Mat2x2&) const;
		Vect2& operator*=(const Mat2x2&);
		float operator*(const Vect2&) const;
		Vect2 operator*(float) const;
		Vect2 operator/(float) const;
		Vect2 operator+(const Vect2&) const;
		Vect2 operator-(const Vect2&) const;
		Vect2& operator+=(const Vect2&);
		float Length() const;
	};

	Mat2x2 rotation_mat2x2(float angle);

	float rad_to_deg(float rad);
	float deg_to_rad(float deg);
	float vec_to_rad(Vect2 v);
	Vect2 rad_to_vec(float rad);
}

namespace utils
{
	int get_thread_number();

	class SimpleCooldown 
	{
	public:
		/// @brief starts timer from now
		/// @param duration : cooldown in milliseconds
		SimpleCooldown(int duration) : m_cooldown(std::chrono::milliseconds(duration))
		{}
		/// @brief resets timer at now
		void reset();
		/// @brief resets timer if cooldown time is reached
		/// @return true if cooldown time is reached
		bool is_ready();

		long long get_time() { return (m_cooldown - (std::chrono::high_resolution_clock::now() - m_lastTime)).count(); }
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<long int, std::milli> m_cooldown;
	};
}


#endif