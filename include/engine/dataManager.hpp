#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include "gameDataStructures.hpp"
#include <memory>
#include <vector>

namespace DataUtils
{
	struct GameData
	{
		GameCameraVars gameCameraVars;
		GameMap gameMap;
		ControlsVars controlsMulti;
		GraphicsVars graphicsVars;
		GameAssets gameAssets;
		std::vector<std::pair<int, std::string>> gameSprites;
	};

	std::unique_ptr<GameData> load_game_data(const std::string&);
}

#endif