#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include"gameDataStructures.hpp"
#include<memory>
#include<vector>

namespace DataUtils
{
	struct GameData
	{
		EntityTransform playerTrasform;
		GameCamera gameCamera;
		GameMap gameMap;
		ControlsVars controlsMulti;
		GraphicsVars screenStats;
		GameAssets gameAssets;
		std::vector<Sprite> gameSprites;
	};

	std::unique_ptr<GameData> load_game_data(const std::string&);
}

#endif