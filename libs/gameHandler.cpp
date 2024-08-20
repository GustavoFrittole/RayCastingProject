#include "gameHandler.hpp"
#include "dataManager.hpp"
#include "gameGraphics.hpp"
#include "gameCore.hpp"
#include "gameInputs.hpp"
#include <iostream>

constexpr auto WINDOW_NAME = "ray cast maze";

#define GENERATION_TIME_STEP_MS 5

class GameHandler : public rcm::IGameHandler
{
public:
	void load_game_data(const std::string&) override;
	void create_assets() override;
	void run_game() override;
private:
	void start();
	void performGameCycle();
	void load_sprites();
	//inline bool goal_reached(const EntityTransform& pos, const GameMap& map);

	std::unique_ptr<DataUtils::GameData> m_gameData;
	std::unique_ptr<GameCore> m_gameCore;
	std::unique_ptr<sf::RenderWindow> m_window;
	std::unique_ptr<GameGraphics> m_gameGraphics;
	std::unique_ptr<InputManager> m_inputManager;
	std::unique_ptr<GameCameraView> m_gameCameraView;
	GameStateVars m_gameState{};
};

rcm::IGameHandler* rcm::create_gameHandler()
{
	return new GameHandler();
}

void GameHandler::load_game_data(const std::string& filePath)
{
	try
	{
		m_gameData = DataUtils::load_game_data(filePath);
	}
	catch (std::exception& e)
	{
		std::string err(e.what());
		err.append("\nError while loading game assets.\n");
		throw std::runtime_error(err);
	}

	m_gameCore = std::make_unique<GameCore>(m_gameData->gameCameraVars, m_gameData->gameMap, m_gameData->playerTrasform);
	m_gameCameraView = std::make_unique<GameCameraView>( GameCameraView{m_gameData->playerTrasform, m_gameData->gameCameraVars, m_gameCore->get_camera_vecs()} );
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(windowVars::g_screenWidth, windowVars::g_screenHeight), WINDOW_NAME);
	m_gameGraphics = std::make_unique<GameGraphics>(*(m_window), m_gameData->windowVars);
	m_inputManager = std::make_unique<InputManager>(m_gameData->controlsMulti, *(m_window), m_gameState, m_gameCore->get_playerController());
}

void GameHandler::create_assets() 
{
	m_gameGraphics->create_assets(m_gameData->gameAssets, m_gameData->gameMap, m_gameData->windowVars, m_gameCore->get_ray_info_arr(), m_gameState, *(m_gameCameraView.get()));
	load_sprites();
}

void GameHandler::run_game()
{
	try
	{
		create_assets();
	}
	catch(std::exception& e)
	{
		std::string err(e.what());
		err.append("\nError while creating game assets.\n");
		throw std::runtime_error(err);
	}

	start();

	//game timer
	debug::GameTimer gt;

	while (m_gameGraphics->is_running())
	{
		performGameCycle();

		//frame counter
		gt.add_frame();
		if (gt.get_frame_count() > 20)
		{
			std::cout << gt.get_frame_rate() << std::endl;
		}
		//
	}
}

void GameHandler::start()
{	
	//step by step map generation (if generation is requested)
	while (m_gameCore->generate_map_step())
	{
		std::thread sleep([] { std::this_thread::sleep_for(std::chrono::milliseconds(GENERATION_TIME_STEP_MS)); });

		m_inputManager->handle_events_close();

		m_gameGraphics->draw_map_gen(m_gameData->gameMap.x, m_gameData->gameMap.y, m_gameData->playerTrasform.coords.x, m_gameData->playerTrasform.coords.y, *(m_gameData->gameMap.cells));

		//wait if the generation time isn't over
		sleep.join();
	}
	m_gameCore->start_internal_time();
	m_gameState.isPaused = true;
}

//inline bool goal_reached(const EntityTransform& pos, const GameMap& map)
//{
//	return (map.cells->at(static_cast<int>(pos.coords.y) * map.x +
//		static_cast<int>(pos.coords.x))
//		== 'g'
//		);
//}

void GameHandler::performGameCycle()
{
	m_window->clear(sf::Color::Black);

	m_inputManager->handle_events_main();

	m_gameCore->update_entities();

	if (m_gameState.isFindPathRequested && m_gameData->gameMap.generated)
	{
		m_gameGraphics->calculate_shortest_path(m_gameData->playerTrasform);
		m_gameState.isFindPathRequested = false;
	}

	m_gameCore->view_by_ray_casting(m_gameState.isLinearPersp);

	m_gameGraphics->draw_view(m_gameState.isLinearPersp, m_gameCore->get_billboards_info_arr());

	if (m_gameState.isPaused || m_gameState.isTabbed)
	{
		m_gameGraphics->draw_map(m_gameData->gameMap.x, m_gameData->gameMap.y, m_gameData->playerTrasform.coords.x, m_gameData->playerTrasform.coords.y, *(m_gameData->gameMap.cells));
		m_gameGraphics->draw_path_out();
	}
	else
	{
		m_gameGraphics->draw_minimap_background(m_gameData->gameMap, m_gameData->playerTrasform, m_gameData->windowVars);
		m_gameGraphics->draw_minimap_triangles(m_gameData->gameCameraVars.pixelWidth, m_gameCore->get_ray_info_arr(), m_gameData->windowVars);

		//if (goal_reached(m_gameData->playerTrasform, m_gameData->gameMap))
		//{
		//	m_gameGraphics->draw_end_screen();
		//}
	}
	m_window->display();
}

void GameHandler::load_sprites()
{
	int id = 0;
	for (const Sprite& s : m_gameData->gameSprites)
	{
		m_gameGraphics->load_sprite(s.texturePath);
		m_gameCore->add_billboard_sprite(id, s.transform);
		++id;
	}
}