#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include"utils.hpp"
#include<memory>
#include<vector>

namespace screenStats
{
	constexpr int g_screenHeight = 720;
	constexpr int g_screenWidth = g_screenHeight * 16 / 9;
}

struct EntityTransform
{
	math::Vect2 coords{};
	float forewardAngle = 0.f;
};

struct GameCamera
{
	int pixelWidth = 0;
	int pixelHeight = 0;
	float fov = 90.f;
	float maxRenderDist = 10.f;
	float rayPrecision = 0.1f;
};

struct Controls
{
	float mouseSens = 1.f;
	float movementSpeed = 1.f;
};

struct ScreenStats
{
	int minimapScale = 6;
	float halfWallHeight = 0.5f;
};

struct GameMap
{
	int x = 0, y = 0;
	bool generated = false;
	std::unique_ptr<std::string> cells;
};

struct GameAssets
{
	std::string wallTexFilePath;
	std::string boundryTexFilePath;
	std::string floorTexFilePath;
	std::string ceilingTexFilePath;
	std::string skyTexFilePath;
};

struct Sprite
{
	std::string texture;
	EntityTransform transform;
};

namespace DataUtils
{
	struct GameData
	{
		EntityTransform playerTrasform;
		GameCamera gCamera;
		GameMap gMap;
		Controls controlsMulti;
		ScreenStats screenStats;
		GameAssets gAssets;
		std::vector<Sprite> gSprites;
	};

	std::unique_ptr<GameData> load_game_data(const std::string&);
}

#endif