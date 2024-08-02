#ifndef GAMEHANDLER_HPP
#define GAMEHANDLER_HPP

#include<string>

namespace rcm
{
	class IGameHandler
	{
	public:
		bool is_good() const { return m_isGood; }
		virtual void run_game() = 0;
		std::string get_errors() const { return m_isGood ? "All good" : m_errors; }
	protected:
		virtual bool load_game_data(const std::string&) = 0;
		bool m_isGood = true;
		std::string m_errors;
	};

	IGameHandler* create_gameHandler(const std::string&);
}

#endif