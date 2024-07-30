
#include"dataManager.hpp"
#include<fstream>
#include<nlohmann/json.hpp>
#include<exception>

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
		gameData->playerTrasform.forewardAngle = data["playerTransform"]["forewardAngleDeg"].get<float>() * (3.14159265358979323846f / 180);
		gameData->playerTrasform.coords.x = data["playerTransform"]["pos"]["x"].get<int>();
		gameData->playerTrasform.coords.y = data["playerTransform"]["pos"]["y"].get<int>();
		//wip
		gameData->gCamera.pixelWidth = screenStats::g_screenWidth;
		gameData->gCamera.pixelHeight = screenStats::g_screenHeight;


		gameData->gCamera.fov = data["gameCamera"]["FOV"].get<float>() * (3.14159265358979323846f / 180);
		gameData->gCamera.maxRenderDist = data["gameCamera"]["maxRenderDist"].get<float>();
		gameData->gCamera.rayPrecision = data["gameCamera"]["rayPrecision"].get<float>();

		gameData->gMap.x = data["gameMap"]["mapW"].get<int>();
		gameData->gMap.y = data["gameMap"]["mapH"].get<int>();
		gameData->gMap.generated = data["gameMap"]["generated"].get<bool>();

		gameData->screenStats.minimapScale = data["screenStats"]["minimapScale"].get<float>();
		gameData->screenStats.halfWallHeight = data["screenStats"]["halfWallHeight"].get<float>();

		mapFilePath = data["gameMap"]["mapCellsFile"].get<std::string>();
	}
	catch (std::exception& e)
	{
		std::string err(e.what());
		err.append("\nError while parsing the configuration file.\n");
		throw std::runtime_error(err);
	}
	try
	{
		load_map_from_file(gameData->gMap.cells, mapFilePath);
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
}

