
#include"dataManager.hpp"
#include<fstream>
#include<nlohmann/json.hpp>
#include<exception>
#include<iostream>

using json = nlohmann::json;

void load_map_from_file(std::unique_ptr<std::string>& tiles, const std::string&);

std::unique_ptr<DataUtils::GameData> DataUtils::load_game_data(const std::string& configPath)
{

	std::ifstream configF(configPath);
	if (!configF.good())
		throw std::invalid_argument("Could not open configuration file.");
	std::unique_ptr<GameData> gameData = std::make_unique<GameData>();
	std::string mapFilePath;

	try
	{
		json data = json::parse(configF);
		gameData->playerTrasform.forewardAngle = data.at("playerTransform").at("forewardAngleDeg").get<float>() * (3.14159265358979323846f / 180);
		gameData->playerTrasform.coords.x = data.at("playerTransform").at("pos").at("x").get<int>();
		gameData->playerTrasform.coords.y = data.at("playerTransform").at("pos").at("y").get<int>();
		//wip
		gameData->gameCamera.pixelWidth = screenStats::g_screenWidth;
		gameData->gameCamera.pixelHeight = screenStats::g_screenHeight;

		gameData->gameCamera.fov = math::deg_to_rad( data.at("gameCamera").at("FOV").get<float>() );
		gameData->gameCamera.maxRenderDist = data.at("gameCamera").at("maxRenderDist").get<float>();
		gameData->gameCamera.rayPrecision = data.at("gameCamera").at("rayPrecision").get<float>();

		gameData->gameMap.x = data.at("gameMap").at("mapW").get<int>();
		gameData->gameMap.y = data.at("gameMap").at("mapH").get<int>();
		gameData->gameMap.generated = data.at("gameMap").at("generated").get<bool>();

		gameData->controlsMulti.mouseSens = data.at("controls").at("mouseSens").get<float>();
		gameData->controlsMulti.movementSpeed = data.at("controls").at("movementSpeed").get<float>();

		gameData->screenStats.minimapScale = data.at("screenStats").at("minimapScale").get<float>();
		gameData->screenStats.halfWallHeight = data.at("screenStats").at("halfWallHeight").get<float>();

		gameData->gameAssets.wallTexFilePath = data.at("textures").at("wallTexPath").get<std::string>();
		gameData->gameAssets.boundryTexFilePath = data.at("textures").at("boundryTexPath").get<std::string>();
		gameData->gameAssets.floorTexFilePath = data.at("textures").at("floorTexPath").get<std::string>();
		gameData->gameAssets.ceilingTexFilePath = data.at("textures").at("ceilingTexPath").get<std::string>();
		gameData->gameAssets.skyTexFilePath = data.at("textures").at("skyTexPath").get<std::string>();

		int spriteNumber = data.at("sprite number").get<int>();
		for (int i = 0; i < spriteNumber; ++i)
		{
			std::string spriteName("sprite");
			spriteName.append(std::to_string(i));
			gameData->gameSprites.push_back({ data.at(spriteName).at("texture").get<std::string>(), EntityTransform{ math::Vect2(data.at(spriteName).at("position").at("x").get<float>() ,data.at(spriteName).at("position").at("y").get<float>())} });
		}


		mapFilePath = data.at("gameMap").at("mapCellsFile").get<std::string>();
		std::cout << data.dump(4);
	}
	catch (std::exception& e)
	{
		std::string err(e.what());
		err.append("\nError while parsing the configuration file.\n");
		throw std::runtime_error(err);
	}
	try
	{
		load_map_from_file(gameData->gameMap.cells, mapFilePath);
	}
	catch (std::exception& e)
	{
		std::string err(e.what());
		err.append("\nCould not load map file.\n");
		throw std::runtime_error(err);
	}
	return gameData;
}

void load_map_from_file(std::unique_ptr<std::string>& tiles, const std::string& mapPath)
{
	std::ifstream file(mapPath);
	if (file.is_open())
	{
		std::string line;
		tiles = std::make_unique<std::string>();
		while (std::getline(file, line))
		{
			*(tiles) += line;
		}
	}
	else
		throw std::runtime_error("Could not open map file.");
}

