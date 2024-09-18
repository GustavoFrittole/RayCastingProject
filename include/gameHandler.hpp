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
		/// @brief Initialize main components using the configuration set in the config file.
		/// @param : path to the config file
		/// @param : player entity that will share its transform with the game camera
		virtual void load_game_data(const std::string&,  std::unique_ptr<IEntity>&) = 0;

		/// @brief Create all game assets (i.e. read and load user defined resources, like textures and fonts, create other components if needed, like the map generetor).
		/// @param entities that will be created before the firts main game cycle
		virtual void create_assets(std::vector<std::unique_ptr<IEntity>>&) = 0;

		/// @brief Function ment to be used in entity scripts. (If used before just appends to the create_assets() entity list).
		/// The entity is created during the same game cycle in which ths function in called, after all actions and interactions are processed, 
		/// before everything else (i.e. it will be seen on camera and it will have its position updated).
		/// @param : entity to be added.
		virtual void add_entity(IEntity* entity) = 0;

		/// @brief Starts the game cycles, beginning with a starting cycle that is run once .
		/// and then proceeding to execute the main cycle untill the end of the cycle in which
		/// close_game() is called.
		virtual void run_game() = 0;

		/// @brief Closes the game at the end of the game cycle in which it is called.
		virtual void close_game() = 0;

		/// @brief Set a string and where to display it on screen. 
		/// The initial position is described with alignments, then an offset can be used for refignments.
		/// @param text : text to be displayed
		/// @param TextVerticalAlignment
		/// @param TextHorizontalAlignment
		/// @param size 
		/// @param offsetX
		/// @param offsetY 
		virtual void set_text_ui(const std::string& text, const TextVerticalAlignment = TextVerticalAlignment::TopWindow, const TextHorizontalAlignment = TextHorizontalAlignment::Left, const int size = -1, const int offsetX = 0, const int offsetY = 0) = 0;

		/// @brief Decide whether the text ui is to be displayed or not.
		/// @return : the current value
		virtual bool& show_text_ui() = 0;

		/// @brief Ment to be used in entity scripts that need input infomation.
		/// @return : a struct containing various values derived from keyboard input, specified in InputCache documentation.
		virtual const InputCache& get_input_cache() = 0;

		/// @brief Check the value of a cell in the given map.
		/// @param pos : position in map
		/// @param map : the map to search in 
		/// @return the char value present in the specified map position
		virtual inline char get_entity_cell(const EntityTransform& pos, const GameMap& map) = 0;

		/// @brief Check the value of a cell in the given map.
		/// @param cellX 
		/// @param cellY 
		/// @param map : the map to search in 
		/// @return : the char value present in the specified map position 
		virtual inline char get_entity_cell(const int cellX, const int cellY, const GameMap& map) = 0;

		/// @brief Get a reference to the current map.
		/// @return a reference to the current map
		virtual inline const GameMap& get_active_map() = 0;
	protected:
		std::string m_configFilePath;
	};

	static std::unique_ptr<IGameHandler> g_gameHandler;

	IGameHandler& get_gameHandler();
}

#endif