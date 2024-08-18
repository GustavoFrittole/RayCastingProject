#ifndef GAMEHANDLER_HPP
#define GAMEHANDLER_HPP

#include <string>

namespace rcm
{
	class IGameHandler
	{
	public:
		IGameHandler() = default;
		virtual void load_game_data(const std::string&) = 0;
		virtual void create_assets() = 0;
		virtual void run_game() = 0;
	protected:
		std::string m_configFilePath;
	};

	IGameHandler* create_gameHandler();
}

#endif