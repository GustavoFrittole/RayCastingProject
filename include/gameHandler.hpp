#ifndef GAMEHANDLER_HPP
#define GAMEHANDLER_HPP

#include <string>
#include <vector>
#include "gameDataStructures.hpp"

#define PROJECTILE_ID 0

namespace rcm
{
	class IGameHandler
	{
	public:
		IGameHandler() = default;
		virtual void load_game_data(const std::string&) = 0;
		virtual void create_assets(const std::vector<Entity>&) = 0;
		virtual void run_game() = 0;
	protected:
		std::string m_configFilePath;
	};

	IGameHandler* create_gameHandler();

	Entity create_projectile(const EntityTransform&);
	//Entity create_target();
	//Entity create_spawner();
}

#endif