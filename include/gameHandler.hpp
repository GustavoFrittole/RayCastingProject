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
		virtual void load_game_data(const std::string&,  std::unique_ptr<IEntity>&) = 0;
		virtual void create_assets(std::vector<std::unique_ptr<IEntity>>&) = 0;
		virtual void add_entity(IEntity* entity) = 0;
		virtual void run_game() = 0;
		virtual void close_game() = 0;
		virtual void set_text_ui(const std::string&, const TextVerticalAlignment = TextVerticalAlignment::TopWindow, const TextHorizontalAlignment = TextHorizontalAlignment::Left, const int size = -1, const int offsetX = 0, const int offsetY = 0) = 0;
		virtual bool& show_text_ui() = 0;
		virtual const InputCache& get_input_cache() = 0;
		virtual inline char get_entity_cell(const EntityTransform& pos, const GameMap& map) = 0;
		virtual inline char get_entity_cell(const int cellX, const int cellY, const GameMap& map) = 0;
		virtual inline const GameMap& get_active_map() = 0;
	protected:
		std::string m_configFilePath;
	};

	static std::unique_ptr<IGameHandler> g_gameHandler;

	IGameHandler& get_gameHandler();
}

#endif