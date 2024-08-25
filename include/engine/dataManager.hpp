#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include "gameDataStructures.hpp"
#include <memory>
#include <vector>

namespace DataUtils
{
	struct GameData
	{
		EntityTransform playerTrasform;
		GameCameraVars gameCameraVars;
		GameMap gameMap;
		ControlsVars controlsMulti;
		GraphicsVars windowVars;
		GameAssets gameAssets;
		std::vector<std::pair<int, std::string>> gameSprites;
	};

	std::unique_ptr<GameData> load_game_data(const std::string&);
}

#endif