#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include "gameDataStructures.hpp"
#include <memory>
#include <vector>

namespace DataUtils
{
	struct GameData
	{
		rcm::GameCameraVars gameCameraVars;
		rcm::GameMap gameMap;
		rcm::ControlsSensitivity controlsMulti;
		rcm::GraphicsVars graphicsVars;
		rcm::GameAssets gameAssets;
		std::vector<std::pair<int, std::string>> gameSprites;
	};

	std::unique_ptr<GameData> load_game_data(const std::string&);
}

#endif