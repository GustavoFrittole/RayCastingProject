
#include "dataManager.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <exception>
#include <iostream>

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
		//wip
		gameData->gameCameraVars.pixelWidth = windowVars::g_windowWidth;
		gameData->gameCameraVars.pixelHeight = windowVars::g_windowHeight;
		//
		gameData->gameCameraVars.fov = math::deg_to_rad( data.at("gameCamera").at("FOV").get<float>() );
		gameData->gameCameraVars.maxRenderDist = data.at("gameCamera").at("maxRenderDist").get<float>();
		gameData->gameCameraVars.rayPrecision = data.at("gameCamera").at("rayPrecision").get<float>();

		gameData->gameMap.width = data.at("gameMap").at("mapW").get<int>();
		gameData->gameMap.height = data.at("gameMap").at("mapH").get<int>();
		gameData->gameMap.generated = data.at("gameMap").at("generated").get<bool>();

		gameData->controlsMulti.mouseSens = data.at("controls").at("mouseSens").get<float>();
		gameData->controlsMulti.movementSpeed = data.at("controls").at("movementSpeed").get<float>();

		gameData->graphicsVars.minimapScale = data.at("windowStats").at("frameRate").get<int>();
		gameData->graphicsVars.minimapScale = data.at("windowStats").at("minimapScale").get<float>();
		gameData->graphicsVars.halfWallHeight = data.at("windowStats").at("halfWallHeight").get<float>();
		gameData->graphicsVars.maxSightDepth = gameData->gameCameraVars.maxRenderDist;
		gameData->graphicsVars.frameRate = data.at("windowStats").at("frameRate").get<int>();

		gameData->gameAssets.fontFilePath = data.at("assets").at("font").get<std::string>();
		gameData->gameAssets.wallTexFilePath = data.at("assets").at("textures").at("wallTexPath").get<std::string>();
		gameData->gameAssets.boundryTexFilePath = data.at("assets").at("textures").at("boundryTexPath").get<std::string>();
		gameData->gameAssets.floorTexFilePath = data.at("assets").at("textures").at("floorTexPath").get<std::string>();
		gameData->gameAssets.ceilingTexFilePath = data.at("assets").at("textures").at("ceilingTexPath").get<std::string>();
		gameData->gameAssets.skyTexFilePath = data.at("assets").at("textures").at("skyTexPath").get<std::string>();

		for (auto& sprite : data.at("sprites"))
			gameData->gameSprites.emplace_back(sprite.at(0).get<int>(), sprite.at(1).get<std::string>());

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

