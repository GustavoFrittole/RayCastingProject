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
		virtual void create_assets(std::vector<std::unique_ptr<IEntity>>&) = 0;
		virtual void add_entity(IEntity* entity) = 0;
		virtual void run_game() = 0;
	protected:
		std::string m_configFilePath;
	};

	static std::unique_ptr<IGameHandler> g_gameHandler;

	IGameHandler& get_gameHandler();

	IEntity* create_projectile(const EntityTransform&);
	//IEntity create_target();
	//IEntity create_spawner();
}

#endif